#include <omp.h>
#include <windows.h>
#include <algorithm>
#include <execution>

#include "Kwire2processor.h"
#include "Kwire2cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace Kwire2 {

	Kwire2Processor::Kwire2Processor()
	{
		//--- set the wanted controller for our processor
		setControllerClass(kKwire2ControllerUID);

		std::fill(rectifiedSignal, rectifiedSignal + MAX_BUFFER_SIZE, 0);

		for (int c = 0; c < 2; ++c)
		{
			filter[c].setMode(TPTSVF::Highpass);
			filter[c].setResonance(0);
		}

		for (int32 id = 0; id < nParams; ++id)
			setParameterNormalised(id, customParameters[id].plainToNormalised(customParameters[id].defaultPlain));
	}

	//------------------------------------------------------------------------
	Kwire2Processor::~Kwire2Processor()
	{
	}

	void Kwire2Processor::setSampleRate(double sr)
	{
		sampleRate = sr;

		for (int c = 0; c < 2; ++c)
		{
			filter[c].setSampleRate(sampleRate);
			distortion[c].setSampleRate(sampleRate);
		}

		updateThreshold = round(updateRate * sampleRate);
	}

	void Kwire2Processor::setParameterNormalised(ParamID id, double value)
	{
		assert(value >= 0.0 && value <= 1.0);
		assert(id >= 0 && id < nParams);

		normalisedValue[id] = value;
		realValue[id] = customParameters[id].normalisedToReal(value);
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::initialize(FUnknown* context)
	{
		// Here the Plug-in will be instantiated

		//---always initialize the parent-------
		tresult result = AudioEffect::initialize(context);
		// if everything Ok, continue
		if (result != kResultOk)
		{
			return result;
		}

		//--- create Audio IO ------
		addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo, Steinberg::Vst::BusTypes::kMain);
		addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo, Steinberg::Vst::BusTypes::kMain);

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::terminate()
	{
		// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

		//---do not forget to call parent ------
		return AudioEffect::terminate();
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::setActive(TBool state)
	{
		//--- called when the Plug-in is enable/disable (On/Off) -----
		return AudioEffect::setActive(state);
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts)
	{
		if (numOuts == 0 || numIns == 0)
			return kResultFalse;

		const int32 inChannels = SpeakerArr::getChannelCount(inputs[0]);

		if (inChannels != 2)
			return kResultFalse;

		if (auto* inBus = FCast<AudioBus>(audioInputs.at(0)))
			inBus->setName(STR16("Stereo In"));

		const int32 outChannels = SpeakerArr::getChannelCount(outputs[0]);

		if (outChannels != 2)
			return kResultFalse;

		if (auto* outBus = FCast<AudioBus>(audioOutputs.at(0)))
			outBus->setName(STR16("Stereo Out"));

		return kResultTrue;
	}

	//------------------------------------------------------------------------

	template<typename SampleType>
	void Kwire2Processor::processAudio(void** in, void** out, const int samples, const double processSampleRate)
	{
		// HP filter and envelope follower
		for (int c = 0; c < 2; ++c)
		{
			const SampleType* inputPtr = static_cast<const SampleType*>(in[c]);

			for (int s = 0; s < samples; ++s)
				amplifiedInput[c][s] = static_cast<double>(inputPtr[s]) * paramValue[inGainId][s];

			// Saturate
			distortion[c].process(amplifiedInput[c], samples);

			for (int s = 0; s < samples; ++s)
			{
				const double cutoff = paramValue[crossoverId][s];

				if (cutoff != filter[c].cutoff)
					filter[c].setCutoff(cutoff);

				filteredInput[c][s] = filter[c].process(amplifiedInput[c][s]);
			}
		}

		// y = 1.0 - ratio * dbtoa(thresholdInDb - atodb(0.5 * (abs(inL) + abs(inR))))
		for (int s = 0; s < samples; ++s)
			rectifiedSignal[s] = abs(filteredInput[0][s]) + abs(filteredInput[1][s]);

		auto& difference = rectifiedSignal;

		for (int s = 0; s < samples; ++s)
			difference[s] = min(0.0, paramValue[thresholdId][s] - atodb(rectifiedSignal[s]));

		auto& attenuation = difference;

		for (int s = 0; s < samples; ++s)
			attenuation[s] = dbtoa(difference[s] * paramValue[ratioId][s]);

		// Slide times
		for (int s = 0; s < samples; ++s)
			attackInSamples[s] = max(1.0, paramValue[attackId][s] * 0.001 * processSampleRate);

		for (int s = 0; s < samples; ++s)
			releaseInSamples[s] = max(1.0, paramValue[releaseId][s] * 0.001 * processSampleRate);

		// Generate envelope
		auto& envelope = attenuation;

		for (int s = 0; s < samples; ++s)
		{
			envelope[s] = slide(attenuation[s], envelopeZ1, attenuation[s] >= envelopeZ1 ? releaseInSamples[s] : attackInSamples[s]);
			envelopeZ1 = envelope[s];
		}

		// Attenuated signal
		for (int c = 0; c < 2; ++c)
		{
			for (int s = 0; s < samples; ++s)
				wetSignal[c][s] = amplifiedInput[c][s] * envelope[s];
		}

		// LR -> MS
		for (int s = 0; s < samples; ++s)
		{
			const double mid = 0.5 * (wetSignal[0][s] + wetSignal[1][s]);
			const double side = 0.5 * (wetSignal[0][s] - wetSignal[1][s]);

			wetSignal[0][s] = mid;
			wetSignal[1][s] = side;
		}

		// Soft-ish clipping
		for (int c = 0; c < 2; ++c)
		{
			for (int s = 0; s < samples; ++s)
			{	
				constexpr double factor = 0.85;
				const double q = max(0.0, paramValue[clipThresholdId][s] - (1.0 - factor));

				const double clamped = std::clamp(wetSignal[c][s], -q, q);
				const double out = clamped + cheapTanh((wetSignal[c][s] - clamped) / (1.0 - factor)) * (1.0 - factor);

				wetSignal[c][s] = (1.0 - paramValue[clipMixId][s]) * wetSignal[c][s] + paramValue[clipMixId][s] * out;
			}
		}

		// MS -> LR
		for (int s = 0; s < samples; ++s)
		{
			const double mid = wetSignal[0][s];
			const double side = wetSignal[1][s];

			wetSignal[0][s] = mid + side;
			wetSignal[1][s] = mid - side;
		}

		// Mix
		// y = mix * outGain * out + (1 - mix) * in
		for (int c = 0; c < 2; ++c)
		{
			for (int s = 0; s < samples; ++s)
				wetSignal[c][s] *= paramValue[outGainId][s];

			for (int s = 0; s < samples; ++s)
				wetSignal[c][s] *= paramValue[mixId][s];

			// Mix takes the untouched input signal (not affected by input gain)
			SampleType* inputPtr = static_cast<SampleType*>(in[c]);

			for (int s = 0; s < samples; ++s)
				inputPtr[s] = static_cast<SampleType>(static_cast<double>(inputPtr[s]) * (1.0 - paramValue[mixId][s]));

			SampleType* outputPtr = static_cast<SampleType*>(out[c]);

			for (int s = 0; s < samples; ++s)
				outputPtr[s] = static_cast<SampleType>(wetSignal[c][s] + static_cast<double>(inputPtr[s]));
		}
	}

	tresult PLUGIN_API Kwire2Processor::process(Vst::ProcessData& data)
	{
		assert(data.processContext != nullptr);

		const int samples = data.numSamples;
		const double processSampleRate = data.processContext->sampleRate;

		assert(MAX_BUFFER_SIZE >= samples);

		for (int32 id = 0; id < nParams; ++id)
			std::fill(paramValue[id], paramValue[id] + MAX_BUFFER_SIZE, realValue[id]);

		if (data.processContext && sampleRate != processSampleRate)
			setSampleRate(processSampleRate);

		if (data.inputParameterChanges)
		{
			int32 numParamsChanged = data.inputParameterChanges->getParameterCount();

			for (int32 index = 0; index < numParamsChanged; index++)
			{
				if (auto* paramQueue = data.inputParameterChanges->getParameterData(index))
				{
					// Make clean point list from parameter queue.
					const ParamID id = paramQueue->getParameterId();

					// Only process incoming data from user parameters.
					if (id >= nParams)
						continue;

					if (paramPointQueue[id].fromParamQueue(paramQueue) <= 0)
						continue;

					auto it = paramPointQueue[id].pointQueue.end();
					--it;

					const ParamValue val = it->second;

					// Long ramp from start to end of the buffer.
					if (val == normalisedValue[id])
					{
						std::fill(paramValue[id], paramValue[id] + MAX_BUFFER_SIZE, realValue[id]);
					}
					else
					{
						for (int s = 0; s < samples; ++s)
						{
							const double t = double(s + 1) / double(samples);
							paramValue[id][s] = customParameters[id].normalisedToReal(herp(normalisedValue[id], val, t));
						}

						setParameterNormalised(id, val);
					}
				}
			}
		}

		void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
		void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);
		const uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, samples);

		if (data.inputs[0].silenceFlags == getChannelMask(data.inputs[0].numChannels))
		{
			// No amplifiedInput, no wetSignal.
			data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

			for (int32 c = 0; c < data.outputs[0].numChannels; c++)
				memset(out[c], 0, sampleFramesSize);
		}
		else
		{
			data.outputs[0].silenceFlags = 0;

			if (data.symbolicSampleSize == Vst::kSample64)
				processAudio<double>(in, out, samples, processSampleRate);
			else if (data.symbolicSampleSize == Vst::kSample32)
				processAudio<float>(in, out, samples, processSampleRate);
		}

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::setupProcessing(Vst::ProcessSetup& newSetup)
	{
		//--- called before any processing ----
		return AudioEffect::setupProcessing(newSetup);
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::canProcessSampleSize(int32 symbolicSampleSize)
	{
		if (symbolicSampleSize == Vst::kSample32)
			return kResultTrue;

		if (symbolicSampleSize == Vst::kSample64)
			return kResultTrue;

		return kResultFalse;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::setState(IBStream* state)
	{
		IBStreamer streamer(state, kLittleEndian);
		std::string paramTitle;

		while (true)
		{
			auto identifier = streamer.readStr8();

			if (!identifier)
				break;

			paramTitle = std::string(identifier);

			CustomParameter* parameter = parameterWithTitle(paramTitle);

			if (!parameter)
				continue;

			double value;
			
			if (!streamer.readDouble(value))
				continue;

			setParameterNormalised(parameter->id, parameter->plainToNormalised(value));
		}

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::getState(IBStream* state)
	{
		IBStreamer streamer(state, kLittleEndian);

		for (CustomParameter& parameter : customParameters)
		{
			if (!streamer.writeStr8(parameter.title.c_str())) return kResultFalse;
			if (!streamer.writeDouble(parameter.normalisedToPlain(normalisedValue[parameter.id]))) return kResultFalse;
		}

		return kResultOk;
	}

	//------------------------------------------------------------------------
}

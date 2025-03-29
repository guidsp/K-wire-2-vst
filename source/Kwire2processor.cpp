#pragma once

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

		for (int p = 0; p < nParams; ++p)
			paramValue[p] = customParameters[p].buffer;

		std::fill(rectifiedSignal, rectifiedSignal + MAX_BUFFER_SIZE, 0);

		filter[0].setMode(TPTSVF::Highpass);
		filter[0].setResonance(0);
		filter[1].setMode(TPTSVF::Highpass);
		filter[1].setResonance(0);
	}

	//------------------------------------------------------------------------
	Kwire2Processor::~Kwire2Processor()
	{
	}

	void Kwire2Processor::setSampleRate(double sr)
	{
		sampleRate = sr;

		filter[0].setSampleRate(sampleRate);
		filter[1].setSampleRate(sampleRate);
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
		addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
		addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

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
		// Support any combination of mono and stereo in/out.

		if (numOuts == 0 || numIns == 0)
			return kResultFalse;

		int32 inChannels = SpeakerArr::getChannelCount(inputs[0]);

		if (auto* inBus = FCast<AudioBus>(audioInputs.at(0)))
		{
			switch (inChannels)
			{
			case 1:
				inBus->setName(STR16("Mono In"));
				break;
			case 2:
				inBus->setName(STR16("Stereo In"));
				break;
			default:
				return kResultFalse;
			}

			inBus->setArrangement(inputs[0]);
		}

		int32 outChannels = SpeakerArr::getChannelCount(outputs[0]);

		if (auto* outBus = FCast<AudioBus>(audioOutputs.at(0)))
		{
			switch (outChannels)
			{
			case 1:
				outBus->setName(STR16("Mono Out"));
				break;
			case 2:
				outBus->setName(STR16("Stereo Out"));
				break;
			default:
				return kResultFalse;
			}

			outBus->setArrangement(outputs[0]);
		}

		return kResultTrue;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::process(Vst::ProcessData& data)
	{
		assert(MAX_BUFFER_SIZE >= data.numSamples);

		for (int p = 0; p < nParams; ++p)
			customParameters[p].prepareBuffer();

		if (data.processContext && sampleRate != data.processContext->sampleRate)
			setSampleRate(data.processContext->sampleRate);

		if (data.inputParameterChanges)
		{
			int32 numParamsChanged = data.inputParameterChanges->getParameterCount();

			for (int32 index = 0; index < numParamsChanged; index++)
			{
				if (auto* paramQueue = data.inputParameterChanges->getParameterData(index))
				{
					// Make clean point list from parameter queue.
					const ParamID paramID = paramQueue->getParameterId();
					const int32 numPoints = paramPointQueue[paramID].fromParamQueue(paramQueue);
					CustomParameter* param = &customParameters[paramID];

					// TODO: Figure out how to handle sample accurate automation with lacking data.
					if (true || numPoints == 1)
					{
						// Received one parameter change point; long ramp from start to end of the buffer.
						auto it = paramPointQueue[paramID].pointQueue.end();
						--it;

						param->update(it->second, data.numSamples);
					}
					else
					{
						// Multiple points; make consecutive ramps between them.
						int32 start = 0;
						int32 end = 0;
						ParamValue startValue = param->normalisedValue;

						for (auto it = paramPointQueue[paramID].pointQueue.begin(); it != paramPointQueue[paramID].pointQueue.end(); ++it)
						{
							end = it->first;

							// A ramp to 0 is an instant step; ignore.
							if (end == 0)
								continue;

							const ParamValue targetValue = it->second;

							if (std::next(it) == paramPointQueue[paramID].pointQueue.end())
							{
								// If this is the last ramp, extend it to the end of the buffer.
								end = data.numSamples - 1;
							}

							param->update(targetValue, start, end);

							start = end;
							startValue = targetValue;
						}

						if (end != (data.numSamples - 1))
							param->update(param->normalisedValue, end, data.numSamples - 1);
					}
				}
			}
		}

		// No amplifiedInput, no wetSignal.
		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

		void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
		void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);

		if (data.symbolicSampleSize == Vst::kSample64)
		{
		}
		else if (data.symbolicSampleSize == Vst::kSample32)
		{
			if (data.outputs[0].numChannels == 2)
			{
				//for (int i = 0; i < data.numSamples; ++i)
				//{
				//	static_cast<float*>(out[0])[i] = paramValue[inGainId][i];
				//	static_cast<float*>(out[1])[i] = paramValue[inGainId][i];
				//}

				//return kResultTrue;


				if (data.inputs[0].numChannels == 2)
				{					
					////////////
					// HP filter and envelope follower

					//#pragma omp parallel
					for (int c = 0; c < 2; ++c)
					{
						std::transform(std::execution::unseq, static_cast<float*>(in[c]), static_cast<float*>(in[c]) + data.numSamples, paramValue[inGainId], amplifiedInput[c],
							[](float input, double gain) { return static_cast<double>(input * gain); });

						for (int s = 0; s < data.numSamples; ++s)
						{
							const double cutoff = paramValue[crossoverId][s];

							if (cutoff != filter[c].cutoff)
								filter[c].setCutoff(cutoff);

							filteredInput[c][s] = filter[c].process(amplifiedInput[c][s]);
						}
					}

					// Envelope follower
					// y = 1.0 - ratio * dbtoa(thresholdInDb - atodb(0.5 * (abs(inL) + abs(inR))))
					std::transform(std::execution::unseq, filteredInput[0], filteredInput[0] + data.numSamples, filteredInput[1], rectifiedSignal,
						[](double left, double right) { return 0.5 * (abs(left) + abs(right)); });

					auto& difference = rectifiedSignal;

					std::transform(std::execution::unseq, rectifiedSignal, rectifiedSignal + data.numSamples, paramValue[thresholdId], difference, 
						[](double signal, double threshold) { return min(0.0, threshold - atodb(signal)); });

					auto& attenuation = difference;

					std::transform(std::execution::unseq, difference, difference + data.numSamples, paramValue[ratioId], attenuation, 
						[](double diff, double ratio) { return dbtoa(diff * ratio); });

					// Slide times
					std::transform(std::execution::unseq, paramValue[attackId], paramValue[attackId] + data.numSamples, attackInSamples,
						[&data](double attackMs) { return max(1.0, attackMs * 0.001 * data.processContext->sampleRate); });

					std::transform(std::execution::unseq, paramValue[releaseId], paramValue[releaseId] + data.numSamples, releaseInSamples,
						[&data](double releaseMs) { return max(1.0, releaseMs * 0.001 * data.processContext->sampleRate); });

					// Apply sliding
					auto& envelope = attenuation;

					for (int s = 0; s < data.numSamples; ++s)
					{
						envelope[s] = slide(attenuation[s], envelopeZ1, attenuation[s] >= envelopeZ1 ? releaseInSamples[s] : attackInSamples[s]);
						envelopeZ1 = envelope[s];
					}

					// Attenuated signal
					// y = in * attenuation
					std::transform(std::execution::unseq, amplifiedInput[0], amplifiedInput[0] + data.numSamples, envelope, wetSignal[0], std::multiplies<double>());
					std::transform(std::execution::unseq, amplifiedInput[1], amplifiedInput[1] + data.numSamples, envelope, wetSignal[1], std::multiplies<double>());
					
					 //Mix
					 //y = mix * outGain * out + (1 - mix) * in
					std::transform(std::execution::unseq, wetSignal[0], wetSignal[0] + data.numSamples, paramValue[outGainId], wetSignal[0], std::multiplies<double>());
					std::transform(std::execution::unseq, wetSignal[1], wetSignal[1] + data.numSamples, paramValue[outGainId], wetSignal[1], std::multiplies<double>());
					
					std::transform(std::execution::unseq, wetSignal[0], wetSignal[0] + data.numSamples, paramValue[mixId], wetSignal[0], std::multiplies<double>());
					std::transform(std::execution::unseq, wetSignal[1], wetSignal[1] + data.numSamples, paramValue[mixId], wetSignal[1], std::multiplies<double>());

					// Mix takes the untouched input signal (not affected by input gain)
					std::transform(std::execution::unseq, (float*)in[0], (float*)in[0] + data.numSamples, paramValue[mixId], (float*)in[0], 
						[](float input, double mix) { return static_cast<float>(input * (1.0 - mix)); });

					std::transform(std::execution::unseq, (float*)in[1], (float*)in[1] + data.numSamples, paramValue[mixId], (float*)in[1],
						[](float input, double mix) { return static_cast<float>(input * (1.0 - mix)); });

					std::transform(std::execution::unseq, wetSignal[0], wetSignal[0] + data.numSamples, (float*)in[0], (float*)out[0],
						[](double wet, float dry) { return static_cast<float>(wet + dry); });

					std::transform(std::execution::unseq, wetSignal[1], wetSignal[1] + data.numSamples, (float*)in[1], (float*)out[1],
						[](double wet, float dry) { return static_cast<float>(wet + dry); });
				}
				else if (data.inputs[0].numChannels == 1)
				{
					// 1 in
					std::transform(std::execution::unseq, (float*)in[0], (float*)in[0] + data.numSamples, paramValue[inGainId], (float*)out[1],
						[](float input, double gain) { return input * float(gain); }
					);
				}
			}
			else if (data.outputs[0].numChannels == 1)
			{
				// 1 out
				if (data.inputs[0].numChannels == 2)
				{
					// 2 ins
					std::transform(std::execution::unseq, (float*)in[0], (float*)in[0] + data.numSamples, (float*)in[1], (float*)out[0], std::plus<float>());
					std::transform(std::execution::unseq, (float*)out[0], (float*)out[0] + data.numSamples, paramValue[inGainId], (float*)out[0],
						[](float input, double gain) { return input * float(gain); }
					);
				}
				else if (data.inputs[0].numChannels == 1)
				{
					// 1 in
					std::transform(std::execution::unseq, (float*)in[0], (float*)in[0] + data.numSamples, paramValue[inGainId], (float*)out[0],
						[](float input, double gain) { return input * float(gain); }
					);
				}
			}
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

			parameter->update(parameter->plainToNormalised(value), MAX_BUFFER_SIZE);
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
			if (!streamer.writeDouble(parameter.normalisedToPlain(parameter.normalisedValue))) return kResultFalse;
		}

		return kResultOk;
	}

	//------------------------------------------------------------------------
}

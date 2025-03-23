#pragma once

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

		std::fill(envelopeFollower, envelopeFollower + MAX_BUFFER_SIZE, 0);
	}

	//------------------------------------------------------------------------
	Kwire2Processor::~Kwire2Processor()
	{
	}

	void Kwire2Processor::setSampleRate(double sr)
	{
		sampleRate = sr;
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

					if (numPoints == 1)
					{
						// Received one parameter change point; long ramp from start to end of the buffer.
						auto it = paramPointQueue[paramID].pointQueue.begin();
						param->update(it->second, data.numSamples);
					}
					else
					{
						// Multiple points; make consecutive ramps between them.
						int32 startSample = 0;
						ParamValue lastValue = param->normalisedValue;

						for (auto it = paramPointQueue[paramID].pointQueue.begin(); it != paramPointQueue[paramID].pointQueue.end(); ++it)
						{
							int32 sampleOffset = it->first;
							ParamValue value = it->second;

							if (std::next(it) == paramPointQueue[paramID].pointQueue.end())
							{
								// If this is the last ramp, linearly extrapolate to the end of the buffer.
								value = std::clamp(lexp(double(data.numSamples - 1), double(startSample), double(it->first), lastValue, value), 0.0, 1.0);
								sampleOffset = data.numSamples - 1;
							}

							param->update(value, startSample, sampleOffset);

							startSample = sampleOffset;
							lastValue = value;
						}
					}
				}
			}
		}

		// No input, no output.
		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

		void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
		void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);

		if (data.symbolicSampleSize == Vst::kSample64)
		{
			if (data.outputs[0].numChannels == 2)
			{
				std::transform(std::execution::unseq, (double*)in[0], (double*)in[0] + data.numSamples, paramValue[inGainId], (double*)out[0], std::multiplies<double>());

				if (data.inputs[0].numChannels == 2)
					std::transform(std::execution::unseq, (double*)in[1], (double*)in[1] + data.numSamples, paramValue[inGainId], (double*)out[1], std::multiplies<double>());
				else
					std::transform(std::execution::unseq, (double*)in[0], (double*)in[0] + data.numSamples, paramValue[inGainId], (double*)out[1], std::multiplies<double>());
			}
			else
			{
				if (data.inputs[0].numChannels == 2)
				{
					std::transform(std::execution::unseq, (double*)in[0], (double*)in[0] + data.numSamples, (double*)in[1], (double*)out[0], std::plus<double>());
					std::transform(std::execution::unseq, (double*)out[0], (double*)out[0] + data.numSamples, paramValue[inGainId], (double*)out[0], std::multiplies<double>());
				}
				else
				{
					std::transform(std::execution::unseq, (double*)in[0], (double*)in[0] + data.numSamples, paramValue[inGainId], (double*)out[0], std::multiplies<double>());
				}
			}
		}
		else
		{
			if (data.outputs[0].numChannels == 2)
			{

				//for (int i = 0; i < data.numSamples; ++i)
				//{
				//	static_cast<float*>(out[0])[i] = paramValue[inGainId][i];
				//	static_cast<float*>(out[1])[i] = paramValue[inGainId][i];
				//}

				//return kResultTrue;

				// 2 outs
				std::transform(std::execution::unseq, (float*)in[0], (float*)in[0] + data.numSamples, paramValue[inGainId], (float*)out[0],
					[](float input, double gain) { return input * float(gain); }
				);

				if (data.inputs[0].numChannels == 2)
				{
					// 2 ins
					std::transform(std::execution::unseq, (float*)in[1], (float*)in[1] + data.numSamples, paramValue[inGainId], (float*)out[1],
						[](float input, double gain) { return input * float(gain); }
					);

					////////////
					// Crossover filter
					
					// Envelope Follower
					// stereo link? maybe just attenuate side by much less
					//envelopeFollower = dbtoa(paramValue[thresholdId] - atodb((abs(out) + abs(out2)) * 0.5));

					// Attenuation
					
					// Mix
				}
				else
				{
					// 1 in
					std::transform(std::execution::unseq, (float*)in[0], (float*)in[0] + data.numSamples, paramValue[inGainId], (float*)out[1],
						[](float input, double gain) { return input * float(gain); }
					);
				}
			}
			else
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
				else
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
		// called when we load a preset, the model has to be reloaded
		IBStreamer streamer(state, kLittleEndian);

		return kResultOk;
	}

	//------------------------------------------------------------------------
	tresult PLUGIN_API Kwire2Processor::getState(IBStream* state)
	{
		// here we need to save the model
		IBStreamer streamer(state, kLittleEndian);

		return kResultOk;
	}

	//------------------------------------------------------------------------
}

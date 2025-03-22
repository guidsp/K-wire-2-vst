#include <windows.h>
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
			value[p] = customParameters[p].buffer;
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
					Vst::ParamValue value;
					int32 sampleOffset;
					const int32 numPoints = paramQueue->getPointCount();

					if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
					{
						const Vst::ParamID paramID = paramQueue->getParameterId();
						CustomParameter* param = &customParameters[paramID];
						param->update(value, data.numSamples);
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
			const auto size = sizeof(double) * data.numSamples;

			if (data.outputs[0].numChannels == 2)
			{
				memcpy(out[0], in[0], size);

				if (data.inputs[0].numChannels == 2)
					memcpy(out[1], in[1], size);
				else
					memcpy(out[1], in[0], size);
			}
			else
			{
				if (data.inputs[0].numChannels == 2)
				{
					//out = in1 + in2;
				}
				else
				{
					memcpy(out[0], in[0], size);
				}
			}
		}
		else
		{
			const auto size = sizeof(float) * data.numSamples;

			if (data.outputs[0].numChannels == 2)
			{
				memcpy(out[0], in[0], size);

				if (data.inputs[0].numChannels == 2)
					memcpy(out[1], in[1], size);
				else
					memcpy(out[1], in[0], size);
			}
			else
			{
				if (data.inputs[0].numChannels == 2)
				{
					//out = in1 + in2;
				}
				else
				{
					memcpy(out[0], in[0], size);
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

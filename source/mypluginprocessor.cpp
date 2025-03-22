//------------------------------------------------------------------------
// Copyright(c) 2025 Laser Brain.
//------------------------------------------------------------------------

#include "mypluginprocessor.h"
#include "myplugincids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"

using namespace Steinberg;

namespace MyCompanyName {
//------------------------------------------------------------------------
// K_wire_2Processor
//------------------------------------------------------------------------
K_wire_2Processor::K_wire_2Processor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kK_wire_2ControllerUID);
}

//------------------------------------------------------------------------
K_wire_2Processor::~K_wire_2Processor ()
{}

void K_wire_2Processor::setSampleRate(double sr)
{
	sampleRate = sr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::process (Vst::ProcessData& data)
{
	if (data.numOutputs == 0)
		return kResultOk;

	if (data.processContext && sampleRate != data.processContext->sampleRate)
		setSampleRate(data.processContext->sampleRate);

    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        
		for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData (index))
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();

				for (int p = 0; p < numPoints; ++p)
				{
					if (paramQueue->getPoint(p, sampleOffset, value) == kResultTrue)
					{
						Vst::ParamID paramID = paramQueue->getParameterId();
						CustomParameter* param = &customParameters[paramID];
						value = param->normalisedToReal(value);
					}
				}

                switch (paramQueue->getParameterId ())
                {
					case inGainId:
						break;
					case crossoverId:
						break;
					case thresholdId:
						break;
					case ratioId:
						break;
					case attackId:
						break;
					case releaseId:
						break;
					case mixId:
						break;
					case outGainId:
						break;
				}
			}
		}
	}
	
	//--- Here you have to implement your processing

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue;

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API K_wire_2Processor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	return kResultOk;
}

//------------------------------------------------------------------------
}

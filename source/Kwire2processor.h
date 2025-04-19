#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include "parameters.h"
#include "ParamPointQueue.h"
#include "TPTSVF.h"
#include "Distortion.h"

namespace Kwire2 {

//------------------------------------------------------------------------
//  Kwire2Processor
//------------------------------------------------------------------------
class Kwire2Processor : public Steinberg::Vst::AudioEffect
{
public:
	Kwire2Processor ();
	~Kwire2Processor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new Kwire2Processor; 
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:
	void setSampleRate(double sr);
	double sampleRate = 44100.0;

	double paramValue[nParams][MAX_BUFFER_SIZE];
	double normalisedValue[nParams] = { 0.0 };
	double realValue[nParams] = { 0.0 };
	ParamPointQueue paramPointQueue[nParams];

	double rectifiedSignal[MAX_BUFFER_SIZE];
	double filteredInput[2][MAX_BUFFER_SIZE];
	double amplifiedInput[2][MAX_BUFFER_SIZE];
	double midSide[2][MAX_BUFFER_SIZE];
	double wetSignal[2][MAX_BUFFER_SIZE];

	double attackInSamples[MAX_BUFFER_SIZE];
	double releaseInSamples[MAX_BUFFER_SIZE];
	double envelopeZ1 = 1.0;

	inline static constexpr double attDecaySpeed = 1.0;

	// Update rate (in seconds) for the non user parameters.
	inline static constexpr double updateRate = 0.016667;
	int updateThreshold = updateRate * 44100.0;

	TPTSVF filter[2];
	Distortion distortion[2];
};

//------------------------------------------------------------------------
} // namespace Kwire2

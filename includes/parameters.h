#pragma once

#include "constants.h"
#include "CustomParameter.h"

//=========================================
// Parameters

enum CCIDs {
	nCCs
};

inline const bool paramIsCC(Steinberg::Vst::ParamID id) {
	return id < nCCs;
}

enum ParameterIDs {
	inGainId = nCCs,
	crossoverId,
	thresholdId,
	ratioId,
	attackId,
	releaseId,
	mixId,
	outGainId,
	nParams
};

static CustomParameter customParameters[nParams] = {
	CustomParameter(inGainId, "Input", "Input", "dB", -24, 24, 0, 0,
	[](double normalised) { return normalised; },
	[](double plain) { return dbtoa(plain); }
	),

	CustomParameter(crossoverId, "Crossover", "Cross", "Hz", 80, 160, 120, 0),

	CustomParameter(thresholdId, "Threshold", "Thresh", "dB", -36, 0, 0, 0,
	[](double normalised) { return normalised; },
	[](double plain) { return dbtoa(plain); }
	),

	CustomParameter(ratioId, "Ratio", "Ratio", ": 1", 1, 10, 2, 0,
	[](double normalised) { return funLog(normalised, -0.2); },
	[](double plain) { return 1.0 - 1.0 / plain; }
	),

	CustomParameter(attackId, "Attack", "Attack", "ms", 0, 100, 10, 0, [](double normalisedValue) { return funLog(normalisedValue, -0.75); }),
	CustomParameter(releaseId, "Release", "Release", "ms", 0, 300, 20, 0, [](double normalisedValue) { return funLog(normalisedValue, -0.75); }),
	CustomParameter(mixId, "Mix", "Mix", "%", 0, 100, 100, 0),

	CustomParameter(outGainId, "Output", "Out", "dB", -24, 24, 0, 0,
	[](double normalised) { return normalised; },
	[](double plain) { return dbtoa(plain); }
	)
};
#pragma once

#include <Windows.h>

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
	clipMixId,
	clipThresholdId,
	mixId,
	outGainId,
	nParams
};

enum InternalParameterIDs {
	nTotalParams = nParams
};

static CustomParameter customParameters[nTotalParams] = {
	CustomParameter(inGainId, "Input", "Input", "dB", -12, 36, 0, 0, 0, [](double plain) { return dbtoa(plain); }),
	CustomParameter(crossoverId, "Crossover", "Cross", "Hz", 10, 800, 120, 0.0, -0.5),
	CustomParameter(thresholdId, "Threshold", "Thresh", "dB", -24, 0, -12),
	CustomParameter(ratioId, "Ratio", "Ratio", "x", 0, 2, 0, 0, -0.5),
	CustomParameter(attackId, "Attack", "Attack", "ms", 0.01, 50, 10, 0, -0.08),
	CustomParameter(releaseId, "Release", "Release", "ms", 5, 200, 25, 0, -0.08),
	CustomParameter(clipMixId, "Clip Mix", "Clip Mix", "%", 0, 100, 0, 0, 0, [](double plain) { return plain * 0.01; }),
	CustomParameter(clipThresholdId, "Clip Threshold", "Clip Thrsh", "dB", -12, 0, 0, 0, 0, [](double plain) { return dbtoa(plain); }),
	CustomParameter(mixId, "Mix", "Mix", "%", 0, 100, 100, 0, 0, [](double plain) { return plain * 0.01; }),
	CustomParameter(outGainId, "Output", "Out", "dB", -24, 24, 0, 0, 0, [](double plain) { return dbtoa(plain); })
};

static CustomParameter* parameterWithTitle(const std::string name)
{
	if (!name.empty())
	{
		for (CustomParameter& parameter : customParameters)
			if (parameter.title == name) return &parameter;
	}

	return nullptr;
}
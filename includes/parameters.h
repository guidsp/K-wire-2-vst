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
	mixId,
	outGainId,
	nParams
};

enum InternalParameterIDs {
	nTotalParams = nParams
};

static CustomParameter customParameters[nTotalParams] = {
	CustomParameter(inGainId, "Input", "Input", "dB", -24, 24, 0, 0, 0, [](double plain) { return dbtoa(plain); }),
	CustomParameter(crossoverId, "Crossover", "Cross", "Hz", 20, 200, 80),
	CustomParameter(thresholdId, "Threshold", "Thresh", "dB", -24, 0, -12),
	CustomParameter(ratioId, "Ratio", "Ratio", "x", 0, 2, 0, 0, -0.5),
	CustomParameter(attackId, "Attack", "Attack", "ms", 0.01, 50, 10, 0, -0.08),
	CustomParameter(releaseId, "Release", "Release", "ms", 1, 200, 25, 0, -0.08),
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
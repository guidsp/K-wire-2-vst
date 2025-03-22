#pragma once

#include <functional>
#include <iomanip>
#include <codecvt>

#include "public.sdk/source/vst/vsteditcontroller.h"

struct CustomParameter {
	CustomParameter(short tag, char* name, char* shortName = "", char* paramUnits = "", 
		double minimum = 0, double maximum = 1, double defaultValue = 1, int stepCount = 0,
		std::function<double(double normalised)> modifier_ = { [=](double normalised) { return normalised; } },
		std::function<double(double normalised)> plainToRealFunc_ = { [=](double plain) { return plain; } },
		
		int32_t paramFlags = Steinberg::Vst::ParameterInfo::ParameterFlags::kCanAutomate) :

		id(tag),
		title(name),
		shortTitle(shortName),
		units(paramUnits),
		minPlain(minimum),
		maxPlain(maximum),
		defaultPlain(defaultValue),
		plainValue(defaultValue),
		steps(stepCount),
		modifier(modifier_),
		plainToRealFunc(plainToRealFunc_),
		flags(paramFlags)
	{
		assert(steps == 0 || steps == int(maxPlain - minPlain));

		range = maxPlain - minPlain;
	};

	short id;

	std::string title,
		shortTitle,
		units;

	double minPlain,
		maxPlain,
		defaultPlain,
		range;

	double plainValue,
		normalisedValue = -1.0,
		realValue;

	int steps;

	int32_t flags;

	// Modifies the distribution. Always works on 0 - 1.
	const std::function<const double(const double normalised)> modifier;
	// Convert the plain value to a working value.
	const std::function<const double(const double plain)> plainToRealFunc;

	//inline const double normalisedToPlain(double normalised) {
	//	//if (normalisedValue == normalised)
	//	//	return plainValue;

	//	return std::fma(modifier(normalisedValue), range, minPlain);
	//}

	inline const double normalisedToReal(double normalised) {
		//if (normalisedValue == normalised)
		//	return realValue;

		return plainToRealFunc(std::fma(modifier(normalisedValue), range, minPlain));
	}
};
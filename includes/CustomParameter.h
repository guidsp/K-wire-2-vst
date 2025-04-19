#pragma once

#include <functional>
#include <iomanip>
#include <codecvt>
#include <cmath>
#include <execution>

#include "public.sdk/source/vst/vsteditcontroller.h"

#include "constants.h"
#include "LookupTable.h"

struct CustomParameter 
{
	CustomParameter(short id_, const char* title_, const char* shortTitle_ = "", const char* units_ = "",
		double min_ = 0, double max_ = 1, double defaultPlain_ = 1, int stepCount_ = 0, double skewFactor_ = 0.0,
		std::function<const double(const double normalised)> plainToRealFunc_ = { [](double plain) { return plain; } },

		int32_t flags_ = Steinberg::Vst::ParameterInfo::ParameterFlags::kCanAutomate) :

		id(id_),
		title(title_),
		shortTitle(shortTitle_),
		units(units_),
		minPlain(min_),
		maxPlain(max_),
		defaultPlain(defaultPlain_),
		range(max_ - min_),
		stepCount(stepCount_),
		skewFactor(skewFactor_),
		plainToReal(plainToRealFunc_),
		flags(flags_)
	{
		assert(stepCount == 0 || stepCount == int(maxPlain - minPlain));

		if (skewFactor != 0.0)
		{
			modifier = [this](double normalised) { return funLog(normalised, skewFactor); };
			reverseModifier = [this](double normalised) { return funLogReverse(normalised, skewFactor); };
		}
	};

	inline const double normalisedToReal(const double normalised) 
	{
		return plainToReal(normalisedToPlain(normalised));
	}

	inline const double normalisedToPlain(const double normalised)
	{
		return minPlain + modifier(normalised) * range;
	}

	inline const double plainToNormalised(const double plain)
	{
		return reverseModifier((std::clamp(plain, minPlain, maxPlain) - minPlain) / range);
	}

	const int id;

	const std::string title,
		shortTitle,
		units;

	const double minPlain,
		maxPlain,
		defaultPlain,
		range;

	// Skew factor applied to the normalised value, changing the
	// distribution of the parameter.
	const double skewFactor;

	const int stepCount;
	const int32_t flags;

	// Convert the plain paramValue to a working paramValue (eg. db to linear gain).
	std::function<const double(const double plain)> plainToReal;

	// Modifies the distribution. Always works on 0 - 1.
	std::function<const double(const double normalised)> modifier = [](double normalised) { return normalised; };

	// Performs the opposite of the modifier function. 
	// Used for normalised -> plain conversion.
	std::function<const double(const double normalised)> reverseModifier = [](double normalised) { return normalised; };
};
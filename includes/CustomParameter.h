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
		plainValue(defaultPlain_),
		stepCount(stepCount_),
		skewFactor(skewFactor_),
		plainToRealFunc(plainToRealFunc_),
		flags(flags_)
	{
		assert(stepCount == 0 || stepCount == int(maxPlain - minPlain));

		if (skewFactor != 0.0)
		{
			// Skew factor; set up the modifier tables and functions.
			//for (auto i = 0; i < wtSize; ++i)
			//{
			//	const double index = double(i) / double(wtSize - 1);

			//	modifierTable.table[i] = funLog(index, skewFactor);
			//	reverseModifierTable.table[i] = funLogReverse(index, skewFactor);
			//}

			//modifier = [this](double normalised) { return modifierTable.lookup(normalised); };
			//reverseModifier = [this](double normalised) { return reverseModifierTable.lookup(normalised); };
			 
			modifier = [this](double normalised) { return funLog(normalised, skewFactor); };
			reverseModifier = [this](double normalised) { return funLogReverse(normalised, skewFactor); };
		}

		normalisedValue = plainToNormalised(plainValue);
		realValue = normalisedToReal(normalisedValue);

		prepareBuffer();
	};

	void prepareBuffer() 
	{
		std::fill(buffer, buffer + MAX_BUFFER_SIZE, realValue);
	}

	// Fills the buffer with a ramp from the previous real paramValue to the new real paramValue.
	inline void update(const double normalised, const int frames) 
	{
		if (normalisedValue == normalised)
		{
			prepareBuffer();
		}
		else
		{
			std::transform(std::execution::unseq, buffer, buffer + frames, buffer, [this, normalised, frames](double& value)
			{
				const auto index = &value - buffer + 1;
				const double t = double(index) / double(frames);
				return normalisedToReal(herp(normalisedValue, normalised, t));
			});

			normalisedValue = normalised;
			realValue = buffer[frames - 1];
		}
	}

	// Creates a ramp from current real paramValue to target paramValue between start and end.
	inline void update(const double normalised, const int start, const int end)
	{
		if (normalisedValue == normalised)
		{
			std::fill(buffer + start, buffer + end + 1, realValue);
		}
		else
		{
			const double range = max(1, end - start);

			std::transform(std::execution::unseq, buffer + start, buffer + end + 1, buffer + start, [this, normalised, start, range](double& value)
			{
				const auto index = &value - (buffer + start) + 1;
				const double t = double(index) / range;

				return normalisedToReal(herp(normalisedValue, normalised, t));
			});

			normalisedValue = normalised;
			realValue = buffer[end];
		}
	}

	inline const double normalisedToReal(const double normalised) 
	{
		return plainToRealFunc(normalisedToPlain(normalised));
	}

	inline const double normalisedToPlain(const double normalised)
	{
		return minPlain + modifier(normalised) * range;
	}

	inline const double plainToNormalised(const double plain)
	{
		return reverseModifier((plain - minPlain) / range);
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

	double plainValue = 0.0,
		normalisedValue = 0.0,
		realValue = 0.0;

	double buffer[MAX_BUFFER_SIZE];

	// Convert the plain paramValue to a working paramValue (eg. db to linear gain).
	std::function<const double(const double plain)> plainToRealFunc;

	// Modifies the distribution. Always works on 0 - 1.
	std::function<const double(const double normalised)> modifier = [](double normalised) { return normalised; };

	// Performs the opposite of the modifier function. 
	// Used for normalised -> plain conversion.
	std::function<const double(const double normalised)> reverseModifier = [](double normalised) { return normalised; };

	//static constexpr int wtSize = 64;

	//// Distribution lookup table. Used if the skew factor != 0.0;
	//LookupTable<double, wtSize> modifierTable;
	//// Its reverse.
	//LookupTable<double, wtSize> reverseModifierTable;
};
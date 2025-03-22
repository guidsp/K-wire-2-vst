#pragma once

#include <functional>
#include <iomanip>
#include <codecvt>
#include <cmath>
#include <execution>

#include "public.sdk/source/vst/vsteditcontroller.h"

#include "constants.h"

struct CustomParameter 
{
	CustomParameter(short id_, const char* title_, const char* shortTitle_ = "", const char* units_ = "",
		double min_ = 0, double max_ = 1, double defaultPlain_ = 1, int stepCount_ = 0,
		std::function<double(double normalised)> modifier_ = { [=](double normalised) { return normalised; } },
		std::function<double(double normalised)> plainToRealFunc_ = { [=](double plain) { return plain; } },

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
		modifier(modifier_),
		plainToRealFunc(plainToRealFunc_),
		flags(flags_)
	{
		assert(stepCount == 0 || stepCount == int(maxPlain - minPlain));

		normalisedValue = (plainValue - minPlain) / range;
		realValue = normalisedToReal(normalisedValue);

		prepareBuffer();
	};

	void prepareBuffer() 
	{
		std::fill_n(buffer, MAX_BUFFER_SIZE, realValue);
	}

	// Fills the buffer with a ramp from the previous real value
	// to the new real value.
	inline void update(const double normalised, const int frames) 
	{
		if (normalisedValue == normalised)
		{
			prepareBuffer();
		}
		else
		{
			// Unary transform; since I provided only one range (first two arguments), the third argument is the output.
			// The lambda gets one argument (double& value), which is a reference to an element of buffer.
			std::transform(std::execution::unseq, buffer, buffer + frames, buffer, [this, normalised, frames](double& value)
			{
				const auto index = &value - buffer;
				double t = double(index) / double(frames - 1);
				return normalisedToReal(std::lerp(normalisedValue, normalised, t));
			});

			normalisedValue = normalised;
			realValue = buffer[frames - 1];

			std::fill(buffer + frames, buffer + MAX_BUFFER_SIZE, realValue);
		}
	}

	inline const double normalisedToReal(double normalised) 
	{
		return plainToRealFunc(minPlain + modifier(normalised) * range);
	}

	const int id;

	const std::string title,
		shortTitle,
		units;

	const double minPlain,
		maxPlain,
		defaultPlain,
		range;

	const int stepCount;
	const int32_t flags;

	double plainValue = 0.0,
		normalisedValue = 0.0,
		realValue = 0.0;

	double buffer[MAX_BUFFER_SIZE];

	// Modifies the distribution. Always works on 0 - 1.
	const std::function<const double(const double normalised)> modifier;
	// Convert the plain value to a working value.
	const std::function<const double(const double plain)> plainToRealFunc;

	//inline const double normalisedToPlain(double normalised) {
	//	//if (normalisedValue == normalised)
	//	//	return plainValue;

	//	return std::fma(modifier(normalisedValue), range, minPlain);
	//}
};
#pragma once
#include <TPTFilter.h>

// The Art of VA Filter Design p. 110
class TPTSVF : public TPTFilter 
{
public:
	enum Mode {
		Lowpass,
		Bandpass,
		Highpass,
		NumModes
	};

	TPTSVF() :
	TPTFilter() 
	{
		setResonance(resonance);

		out[Lowpass] = &TPTSVF::lowpass;
		out[Bandpass] = &TPTSVF::bandpass;
		out[Highpass] = &TPTSVF::highpass;
	};

	inline double process(double input) override 
	{
		const double feedback = i1s * (g + R2) + i2s;

		i1x = h * (input - feedback);

		i1y = i1x * g + i1s;
		i1s = i1x * g + i1y;

		i2y = i1y * g + i2s;
		i2s = i1y * g + i2y;

		return (this->*out[mode])();
	}
	
	inline double lowpass() override { return i2y; }
	inline double bandpass() { return i1y; }
	inline double highpass() override { return i1x; }

	void reset() override {
		i1x = i1s = i1y = i2s = i2y = 0;
	}

	inline void setCutoff(double cutoffFrequency) override 
	{
		cutoff = std::clamp(cutoffFrequency, 5.0, 20000.0);

		g = tan(M_PI * cutoff * inverseSampleRate);

		updateCoefficients();
	}

	double resonance = 0.1;

	/// Set cutoff frequency and resonance.
	inline void setResonance(double value) 
	{
		resonance = value;
		R2 = 2.0 - 2.0 * resonance;
		
		updateCoefficients();
	}

	virtual void updateCoefficients() 
	{
		h = 1.0 / (1.0 + R2 * g + g * g);
	}

protected:
	typedef double (TPTSVF::* Output)();
	Output out[NumModes];

	double R2 = 0,
		h = 0;

	double k = 0,
		i2s = 0,
		i2y = 0;
};
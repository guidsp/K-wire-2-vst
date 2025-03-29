#pragma once
#define _USE_MATH_DEFINES
#include <algorithm>
#include <math.h>

// Trapezoidal integrator.
class TPTFilter {
public:
	enum Mode {
		Lowpass,
		Highpass,
		Allpass,
		Lowshelf,
		Highshelf,
		NumModes
	};

	TPTFilter() 
	{
		setCutoff(20000.0);

		out[Lowpass] = &TPTFilter::lowpass;
		out[Highpass] = &TPTFilter::highpass;
		out[Allpass] = &TPTFilter::allpass;
		out[Highshelf] = &TPTFilter::highshelf;
		out[Lowshelf] = &TPTFilter::lowshelf;
	};

	virtual void setSampleRate(double samplerate) 
	{
		inverseSampleRate = 1.0 / samplerate;
		maxCutoffFrequency = (samplerate / 2.0) - 1.0;

		setCutoff(cutoff);
		reset();
	}

	virtual void reset() 
	{
		i1s = 0;
	}

	virtual inline double process(double input) 
	{
		i1x = input;
		i1y = (input - i1s) * g + i1s;
		//i1s = (input - i1s) * g + i1y;
		i1s = i1y + i1y - i1s;
		return (this->*out[mode])();
	}

	virtual inline double lowpass() { return i1y; }
	virtual inline double highpass() { return i1x - lowpass(); }
	virtual inline double allpass() { return lowpass() - highpass(); }
	virtual inline double highshelf() { return lowpass() + gain * highpass(); }
	virtual inline double lowshelf() { return gain * lowpass() + highpass(); }

	double cutoff = 1000;

	virtual inline void setCutoff(double cutoffFrequency) 
	{
		cutoff = std::clamp(cutoffFrequency, 5.0, maxCutoffFrequency);

		const double w0 = tan(M_PI * cutoff * inverseSampleRate); // Omega factor.
		g = w0 / (w0 + 1.0); // Value binding between 0 - 1 from 0 to nyquist.
	}

	virtual void setMode(int filterMode) 
	{
		assert(filterMode < NumModes);

		mode = filterMode;
	}

	// Coefficients
	double g = 0;

	// Gain for shelf filters
	double gain = 1.0;

protected:
	int mode = Lowpass;

	double inverseSampleRate = 1.0 / 44100.0;
	double maxCutoffFrequency = (44100.0 / 2.0) - 1;

	double i1x = 0,
		i1s = 0,
		i1y = 0;

private:
	typedef double (TPTFilter::* Output)();
	Output out[NumModes];
};
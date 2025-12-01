#pragma once
#include <algorithm>
#include <constants.h>

// https://www.desmos.com/calculator/fagrsqzigt

class Distortion {
public:
	Distortion() 
	{
		setSampleRate(44100);
	};

	void setSampleRate(double samplerate) 
	{
		sampleRate = samplerate;

		setDriveTime(driveTime);

		std::fill(factor, factor + MAX_BUFFER_SIZE, 0.0);
		std::fill(dry, dry + MAX_BUFFER_SIZE, 0.0);
	}

	void setDriveTime(double ms) 
	{
		driveTime = ms;
		driveTimeSamps = driveTime * sampleRate * 0.001;
	}

	inline void process(double* input, int numSamples) 
	{
		for (int s = 0; s < numSamples; ++s)
		{
			env0 = slide(abs(input[s]), env0Z1, driveTimeSamps);
			env0Z1 = env0;

			factor[s] = input[s] + min(env0, 1.4);
			dry[s] = min(1.0, 3.2 * env0);
		}

		for (int s = 0; s < numSamples; ++s)
			input[s] = dry[s] * input[s] + (1.0 - dry[s]) * input[s] * (27.0 + factor[s] * input[s]) / (27.0 + 9.0 * factor[s] * input[s]);
	}

private:
	double sampleRate = 44100;
	double driveTime = 113,
		driveTimeSamps = 113 * 44100.0 * 0.001;

	double env0 = 0,
		env0Z1 = 0;

	double factor[MAX_BUFFER_SIZE],
		dry[MAX_BUFFER_SIZE];
};
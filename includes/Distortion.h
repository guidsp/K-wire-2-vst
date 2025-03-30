#pragma once
#include <algorithm>

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
	}

	void setDriveTime(double ms) 
	{
		driveTime = ms;
		driveTimeSamps = driveTime * sampleRate * 0.001;
	}

	inline double process(double input) 
	{
		env0 = slide(abs(input), env0Z1, driveTimeSamps);
		env0Z1 = env0;

		const double factor = input + min(env0, 1.4);
		const double dry =  min(1.0, 3.2 * env0);

		return dry * input + (1.0 - dry) * input * (27.0 + factor * input) / (27.0 + 9.0 * factor * input);
	}

private:
	double sampleRate = 44100;
	double driveTime = 113,
		driveTimeSamps = 113 * 44100.0 * 0.001;

	double env0 = 0,
		env0Z1 = 0;
};
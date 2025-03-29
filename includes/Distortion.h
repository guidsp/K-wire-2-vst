#pragma once
#include <algorithm>

// https://www.desmos.com/calculator/fagrsqzigt

class Distortion {
public:
	Distortion() {
		setSampleRate(44100);
	};

	void setSampleRate(double samplerate) {
		sampleRate = samplerate;

		setDriveTime(driveTime);
	}

	void setDriveTime(double ms) {
		driveTime = ms;
		driveTimeSamps = driveTime * sampleRate * 0.001;
	}

	inline double process(double input) 
	{
		env0 = slide(input, env0Z1, driveTimeSamps);
		env0Z1 = env0;

		const double factor = input + min(env0, 1.4);

		return input * (27.0 + factor * input) / (27.0 + 9.0 * factor * input);
	}

private:
	double sampleRate = 44100;
	double driveTime = 200.0,
		driveTimeSamps = 200.0 * 44100.0 * 0.001;

	double env0 = 0,
		env0Z1 = 0;
};
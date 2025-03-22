//------------------------------------------------------------------------
// Copyright(c) 2025 Laser Brain.
//------------------------------------------------------------------------

#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <cassert>
#include <debugapi.h>
#include <string>

//=========================================
// Constants

#ifndef MATH_CONSTANTS
#define M_PI 3.14159265358979323846264338327950288
#define M_SQRT2 1.4142135623730950488016887
#define MATH_CONSTANTS
#endif

static constexpr int NUM_CHANNELS = 2;
static constexpr int MAX_BUFFER_SIZE = 4096;
static constexpr double PI = M_PI;

//=========================================
// Constants

// Adjustable curve for inputs between 0 and 1. Negative curve is exponential, positive curve above 1 is logarithmic.
// https://www.desmos.com/calculator/bepz0uj5b9
inline static double funLog(const double input, const double curve = -0.25) {
	assert(!std::isinf(curve * (input / (-1.0 + curve + input))));

	return curve * (input / (-1.0 + curve + input));
}

template <typename T>
inline T dbtoa(const T dB) {
	static constexpr double DB_2_LOG = 0.11512925464970228420089957273422;	// ln( 10 ) / 20
	return exp(dB * DB_2_LOG);
}

template <typename T>
inline T atodb(const T gain) {
	static constexpr double LOG_2_DB = 8.6858896380650365530225783783321;	// 20 / ln( 10 )
	return std::max(-120.0, log(lin) * LOG_2_DB);
}

template <typename T>
inline T mtof(const T note) {
	return 440.0 * std::pow(2.0, ((note - 69.0) * (1.0 / 12.0)));
}

template <typename T>
inline T ftom(const T f) {
	return 12.0 * (log(f / 220.0) * 3.32192809489) + 57.0;
}

inline static void debug(double value) {
	OutputDebugStringA(std::to_string(value).c_str());
	OutputDebugStringA("\n");
}

inline static void debug(const char* str) {
	OutputDebugStringA(str);
	OutputDebugStringA("\n");
}
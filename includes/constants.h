#pragma once

#include <cmath>
#include <cassert>
#include <debugapi.h>
#include <string>

//=========================================
// Constants

static constexpr int MAX_CHANNELS = 2;
static constexpr int MAX_BUFFER_SIZE = 4096;

//=========================================
// Functions

// Adjustable curve for inputs between 0 and 1. Negative curve is exponential, positive curve above 1 is logarithmic.
// https://www.desmos.com/calculator/bepz0uj5b9
inline static double funLog(const double input, const double curve = -0.25) 
{
	assert(!std::isinf(curve * (input / (-1.0 + curve + input))));

	return curve * (input / (-1.0 + curve + input));
}

template <typename T>
inline static T dbtoa(const T dB) 
{
	static constexpr double DB_2_LOG = 0.11512925464970228420089957273422;	// ln( 10 ) / 20
	return exp(dB * DB_2_LOG);
}

template <typename T>
inline static T atodb(const T gain) 
{
	static constexpr double LOG_2_DB = 8.6858896380650365530225783783321;	// 20 / ln( 10 )
	return max(-120.0, log(gain) * LOG_2_DB);
}

template <typename T>
inline static T mtof(const T note)
{
	return 440.0 * std::pow(2.0, ((note - 69.0) * (1.0 / 12.0)));
}

template <typename T>
inline static T ftom(const T f)
{
	return 12.0 * (log(f / 220.0) * 3.32192809489) + 57.0;
}

inline static void debug(double value) 
{
	OutputDebugStringA(std::to_string(value).c_str());
	OutputDebugStringA("\n");
}

inline static void debug(const char* str) 
{
	OutputDebugStringA(str);
	OutputDebugStringA("\n");
}

inline static void debug(const char* str, double value) 
{
	std::string output = str;
	output += ": ";
	output += std::to_string(value);
	output += "\n";
	OutputDebugStringA(output.c_str());
}
#pragma once

#include <cmath>
#include <cassert>
#include <debugapi.h>
#include <string>
#include <stdexcept>

//=========================================
// Constants

static constexpr int MAX_BUFFER_SIZE = 4096;
static constexpr int DISPLAY_VALUE_COUNT = 16;

//=========================================
// Functions

// Adjustable curve for inputs between 0 and 1. Negative curve is exponential, positive curve above 1 is logarithmic.
// https://www.desmos.com/calculator/bepz0uj5b9
template <typename T>
inline static T funLog(const T input, const T curve = -0.25) 
{
	assert(!std::isinf(curve * (input / (-1.0 + curve + input))));

	return curve * (input / (-1.0 + curve + input));
}

template <typename T>
inline static T funLogReverse(const T input, const T curve = -0.25)
{
	assert(!std::isinf((input - input * curve) / (input - curve)));

	return (input - input * curve) / (input - curve);
}

template <typename T>
inline static T saturate(const T input, const T factor)
{
	assert(factor >= 0.1 && factor <= 1.0);

	return input / ((1.0 - factor) * input * input + factor);
}

// y = z1 + (x - z1) / steps
template <typename T>
inline static T slide(const T input, const T prevOutput, const T steps) {
	return prevOutput + (input - prevOutput) / steps;
}

// Hermite interpolation with a fixed slope, often called smoothstep.
template <typename T>
inline static T herp(const T first, const T second, double i)
{
	const double weight = i * i * (3.0 - 2.0 * i);

	return (1.0 - weight) * first + weight * second;
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

static std::u16string toU16String(const std::string& narrowString) 
{
	// Calculate the size needed for the UTF-16 string
	int wideSize = MultiByteToWideChar(CP_UTF8, 0, narrowString.c_str(), -1, nullptr, 0);

	if (wideSize == 0)
		return u"";

	// Allocate a buffer for the UTF-16 string
	std::vector<char16_t> wideBuffer(wideSize);
	MultiByteToWideChar(CP_UTF8, 0, narrowString.c_str(), -1, reinterpret_cast<LPWSTR>(wideBuffer.data()), wideSize);

	// Return the UTF-16 string
	return std::u16string(wideBuffer.data());
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
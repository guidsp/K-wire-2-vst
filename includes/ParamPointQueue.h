#pragma once

using namespace Steinberg;
using namespace Steinberg::Vst;

struct ParamPointQueue
{
public:
	// This function creates a clean point list from an IParamValueQueue,
	// meaning no elements share the same sampleOffset.
	int32 fromParamQueue(Steinberg::Vst::IParamValueQueue* paramQueue)
	{
		assert(paramQueue);

		pointQueue.clear();

		const int32 points = paramQueue->getPointCount();

		for (int32 i = 0; i < points; ++i)
		{
			int32 sampleOffset;
			ParamValue value;

			// Overwrite previous values at the same offset
			if (paramQueue->getPoint(i, sampleOffset, value) == Steinberg::kResultOk)
				pointQueue[sampleOffset] = value;
		}

		numPoints = pointQueue.size();

		return numPoints;
	}

	std::map<int32, ParamValue> pointQueue;
	int32 numPoints = 0;
};
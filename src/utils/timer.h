#pragma once
#include <chrono>

namespace utils
{
	class Timer
	{
	private:
		std::chrono::time_point<std::chrono::steady_clock> StartTimePoint;
	public:
		inline void Start()
		{
			StartTimePoint = std::chrono::steady_clock::now();
		}

		inline float GetElapsedTime()
		{
			auto currentTime = std::chrono::steady_clock::now();
			std::chrono::duration<float, std::milli> duration = currentTime - StartTimePoint;

			return duration.count();
		}
	};
}
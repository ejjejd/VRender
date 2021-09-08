#pragma once
#include "logger.h"
#include "utils/timer.h"

#ifndef NDEBUG
	#define LOGC(...) debug::GlobalLoggger.Send(debug::LogSeverity::Common, __VA_ARGS__)
	#define LOGW(...) debug::GlobalLoggger.Send(debug::LogSeverity::Warning, __VA_ARGS__)
	#define LOGE(...) debug::GlobalLoggger.Send(debug::LogSeverity::Error, __VA_ARGS__)

	#define ASSERT(c, ...) if(!(c)) { LOGE(__VA_ARGS__); std::abort(); }


	inline utils::Timer GlobalTimer;

	#define START_TIMER() GlobalTimer.Start()
	#define END_TIMER() LOGW("%f", GlobalTimer.GetElapsedTime())
#else
	#define LOGC(...)
	#define LOGW(...)
	#define LOGE(...)

	#define ASSERT(c, ...)

	#define START_TIMER()
	#define END_TIMER()
#endif
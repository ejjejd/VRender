#pragma once
#include "logger.h"
#include "utils/timer.h"

#define FORCE_SEMICOLON_BLOCK(x)\
	do { x } while(false)

#ifndef NDEBUG
	#define LOGC(...) debug::GlobalLoggger.Send(debug::LogSeverity::Common, __VA_ARGS__)
	#define LOGW(...) debug::GlobalLoggger.Send(debug::LogSeverity::Warning, __VA_ARGS__)
	#define LOGE(...) debug::GlobalLoggger.Send(debug::LogSeverity::Error, __VA_ARGS__)

	#define ASSERT(c, ...) FORCE_SEMICOLON_BLOCK(if(!(c)) { LOGE(__VA_ARGS__); std::abort(); })


	inline utils::Timer GlobalTimer;

	#define START_TIMER() GlobalTimer.Start()
	#define END_TIMER() LOGW("%f", GlobalTimer.GetElapsedTime())
#else
	#define LOGC(...)
	#define LOGW(...)
	#define LOGE(...)
	
	//Condition should be called because a lot of important functions is called in it
	#define ASSERT(c, ...) FORCE_SEMICOLON_BLOCK(if(!(c)) { }) 

	#define START_TIMER()
	#define END_TIMER()
#endif
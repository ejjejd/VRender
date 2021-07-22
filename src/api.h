#pragma once

#ifdef LIB
	#define API __declspec(dllexport)
#else
	#define API __declspec(dllimport)
#endif
#pragma once
#include <cstdlib>

#define LOG(...) { printf(__VA_ARGS__); printf("\n"); }
#define TERMINATE_LOG(...) { printf(__VA_ARGS__); std::abort(); }
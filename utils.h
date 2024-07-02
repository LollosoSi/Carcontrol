#pragma once

#include <time.h> // CLOCK_MONOTONIC_RAW, timespec, clock_gettime()
#include <chrono>
using namespace std::chrono;
uint64_t currentMillis() { return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count(); }

long mapvalues(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
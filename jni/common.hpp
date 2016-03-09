#ifndef COMMON_H
#define COMMON_H

#include <chrono>

void doLog(const char* text);
void diffclock(char const *title, clock_t clock2);
void doLogClock(const char* format, const char* title, double diffms);

#define DEBUG_LEVEL 3
#define DEBUG_TAG "NDK_AndroidNDK1SampleActivity"

#define IS_PHONE

#ifdef IS_PHONE
#include <jni.h>
#endif

#ifdef IS_PHONE
extern JNIEnv* env;
#endif

extern int PHONE;
extern std::chrono::high_resolution_clock::time_point startx;

#endif

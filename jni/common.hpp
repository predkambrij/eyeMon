#ifndef COMMON_H
#define COMMON_H

#include <chrono>

void doLog(const char* text);
void diffclock(char const *title, clock_t clock2);
void difftime(char const *title, std::chrono::time_point<std::chrono::steady_clock> t2);
void doLogClock(const char* format, const char* title, double diffms);
void doLogClock1(const char* format, const char* title, long int diffms);

#define DEBUG_LEVEL 3
#define DEBUG_TAG "NDK_AndroidNDK1SampleActivity"

#define METHOD_OPTFLOW 0
#define METHOD_TEMPLATE_BASED 1
#define METHOD_BLACK_PIXELS 2


//#define IS_PHONE

#ifdef IS_PHONE
#include <jni.h>
#endif

#ifdef IS_PHONE
extern JNIEnv* env;
#endif

extern int PHONE;
extern std::chrono::high_resolution_clock::time_point startx;
extern int method;

extern bool debug_show_img;
extern bool debug_show_img_optfl_eyes;

#endif

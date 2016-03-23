#ifndef COMMON_H
#define COMMON_H

#include <chrono>
#include <list>
#include <blinkmeasure.hpp>

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
extern int pause;

extern bool debug_show_img_main;
extern bool debug_show_img_face;
extern bool debug_show_img_optfl_eyes;
extern bool debug_show_img_templ_eyes_cor;
extern bool debug_show_img_templ_eyes_tmpl;
extern bool debug_t1_log;
extern bool debug_t2_log;
extern bool debug_t2_perf_method;
extern bool debug_t2_perf_whole;
extern bool debug_tmpl_log;
extern bool debug_tmpl_perf1;
extern bool debug_tmpl_perf2;
extern bool debug_blinks_d1;
extern bool debug_blinks_d2;
// optical flow
extern int flg;
///


extern std::list<BlinkMeasure> blinkMeasure;
extern std::list<BlinkMeasure> blinkMeasureShort;

#endif

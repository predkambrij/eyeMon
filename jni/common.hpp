#ifndef COMMON_H
#define COMMON_H

#include <chrono>
#include <list>

#include <opencv2/core/core.hpp>

#include <blinkmeasure.hpp>

void doLog(bool shouldPrint, const std::string fmt, ...);
void diffclock(char const *title, clock_t clock2);
void difftime(char const *title, std::chrono::time_point<std::chrono::steady_clock> t2, bool shouldExecute);
void doLogClock(const char* format, const char* title, double diffms);
void doLogClock1(const char* format, const char* title, long int diffms);
void imshowWrapper(const char* name, cv::Mat mat, bool shouldShow);
void printStatus();

#define DEBUG_LEVEL 3
#define DEBUG_TAG "NDK_AndroidNDK1SampleActivity"

#define METHOD_OPTFLOW 0
#define METHOD_TEMPLATE_BASED 1
#define METHOD_BLACK_PIXELS 2

extern int PHONE;
extern std::chrono::high_resolution_clock::time_point startx;
extern int method;
extern int pause;
extern double previousFrameTime;

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
extern bool debug_notifications_log1;
extern bool debug_blinks_d1;
extern bool debug_blinks_d2;
extern bool debug_blinks_d3;
extern bool debug_blinks_d4;
// optical flow
extern int flg;

extern int farne;

extern int leftXOffset, leftYOffset, leftCols, leftRows;
extern int rightXOffset, rightYOffset, rightCols, rightRows;

extern int leftXp1, leftYp1, rightXp1, rightYp1;
extern int leftXlast, leftYlast, rightXlast, rightYlast;
extern int leftXavg, leftYavg, rightXavg, rightYavg;
extern int eye_region_width, eye_region_height;

extern cv::TermCriteria termcrit;
extern cv::Size subPixWinSize, winSize;
extern const int MAX_COUNT;
extern bool addRemovePtx;
extern cv::vector<cv::Point2f> points[2];
extern cv::Mat pleft, pright;
extern cv::Mat toSave;
extern int firstLoopProcs;

///
extern int maxFramesShortList;


extern std::list<BlinkMeasure> blinkMeasure;
extern std::list<BlinkMeasure> blinkMeasureShort;
extern std::list<Blink> lBlinkChunks;
extern std::list<Blink> rBlinkChunks;
extern std::list<Blink> lBlinkTimeframeChunks;
extern std::list<Blink> rBlinkTimeframeChunks;

#endif

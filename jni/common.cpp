#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <chrono>

#include <stdarg.h>  // For va_start, etc.

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <common.hpp>

#ifdef IS_PHONE
#include <android/log.h>
#endif

#ifdef IS_PHONE
JNIEnv* env;
#endif
/*
// TODO this could be done using macros (at all places is used ifdef)
bool debug_show_img_main = false;
bool debug_show_img_face = false;
bool debug_show_img_optfl_eyes = false;
bool debug_show_img_templ_eyes_cor = false;
bool debug_show_img_templ_eyes_tmpl = false;
bool debug_t1_log = false;
bool debug_t2_log = false;
bool debug_t2_perf_method = false;
bool debug_t2_perf_whole = true;
bool debug_tmpl_log = false;
bool debug_tmpl_perf1 = false;
bool debug_tmpl_perf2 = false;
bool debug_notifications_log1 = false;
bool debug_blinks_d1 = false;
bool debug_blinks_d2 = false;
bool debug_blinks_d3 = false;

//int method = METHOD_OPTFLOW;
int method = METHOD_TEMPLATE_BASED;
double previousFrameTime = -1;

// optical flow
int flg=0;
///
*/

std::list<BlinkMeasure> blinkMeasure;
std::list<BlinkMeasure> blinkMeasureShort;
std::list<Blink> lBlinkChunks;
std::list<Blink> rBlinkChunks;
std::list<Blink> lBlinkTimeframeChunks;
std::list<Blink> rBlinkTimeframeChunks;
int pause = 0;

int PHONE = 1;
std::chrono::high_resolution_clock::time_point startx = std::chrono::high_resolution_clock::now();

void printStatus() {

}

/**
 * CPU time
 * clock_t loopStart = clock();
 * diffclock("frame capture:", loopStart);
 */
void diffclock(char const *title, clock_t clock2) {
    clock_t clock1 = clock();
    double diffticks = double(clock1 - clock2);
    double diffms    = diffticks * 1000.0 / CLOCKS_PER_SEC;

    doLogClock("%s: %f", title, diffms);
}

/**
 * Actualtime
 * std::chrono::time_point<std::chrono::steady_clock> start1 = std::chrono::steady_clock::now();
 * difftime("frame capture:", loopStart);
 */
void difftime(char const *title, std::chrono::time_point<std::chrono::steady_clock> t1, bool shouldExecute) {
    if (shouldExecute != true) {
        return;
    }
    std::chrono::time_point<std::chrono::steady_clock> t2 = std::chrono::steady_clock::now();
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

    doLogClock1("PERF %s: %ld", title, duration.count());
}

void doLogClock(const char* format, const char* title, double diffms) {
    doLog(true, format, title, diffms);
}

void doLogClock1(const char* format, const char* title, long int diffms) {
    doLog(true, format, title, diffms);
}

void doLog(bool shouldPrint, const std::string fmt, ...) {
    if (shouldPrint != true) {
        return;
    }
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            break;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }

#ifndef IS_PHONE
    std::ofstream myfile;
    myfile.open("/tmp/testlog.txt", std::fstream::app);
    //myfile << "Writing this to a file.\n";
    myfile << str << std::endl ;
    myfile.close();// does at destruction
    //myfile.flush();
    //std::cout << str << std::endl ;
#else
    char text[str.size()+1];//as 1 char space for null is also required
    strcpy(text, str.c_str());

    // malloc room for the resulting string
    char *szResult;
    szResult = (char*)malloc(sizeof(char)*(str.size()+100));

    auto end = std::chrono::high_resolution_clock::now();
    unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-startx).count();

    // standard sprintf
    sprintf(szResult, "res: %llu %s", ns, text);

    __android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, "NDK:OPTFLOF: [%s]", szResult);

    // cleanup
    free(szResult);
#endif
}

void imshowWrapper(const char* name, cv::Mat mat, bool shouldShow) {
#ifndef IS_PHONE
    if (shouldShow == true) {
        imshow(name, mat);
    }
#endif
}
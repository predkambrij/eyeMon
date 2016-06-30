#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include <mutex>

#include <stdarg.h>  // For va_start, etc.

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <common.hpp>

#ifdef IS_PHONE
#include <android/log.h>
#endif

/// farne
int maxFramesShortListf = 0;
std::list<BlinkMeasureF> blinkMeasuref;
std::list<BlinkMeasureF> blinkMeasureShortf;
std::list<BlinkF> lBlinkChunksf;
std::list<BlinkF> rBlinkChunksf;
std::list<BlinkF> lBlinkTimeframeChunksf;
std::list<BlinkF> rBlinkTimeframeChunksf;
///

std::list<BlinkMeasure> blinkMeasure;
std::list<BlinkMeasure> blinkMeasureShort;
std::list<Blink> lBlinkChunks;
std::list<Blink> rBlinkChunks;
std::list<Blink> lBlinkTimeframeChunks;
std::list<Blink> rBlinkTimeframeChunks;
int pause = 0;
cv::Mat toSave;

cv::CascadeClassifier face_cascade;

int PHONE = 1;
int maxFramesShortList = 0;
std::chrono::high_resolution_clock::time_point startx = std::chrono::high_resolution_clock::now();

std::mutex logMutex;

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

    doLogClock("%s: %f\n", title, diffms);
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
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    long int cnt = duration.count();
    double mils = cnt/((double)1000);
    doLogClock1("PERF %s: %.3lf\n", title, mils);
}

void doLogClock(const char* format, const char* title, double diffms) {
    doLog(true, format, title, diffms);// TODO newline?
}

void doLogClock1(const char* format, const char* title, double diffms) {
    doLog(true, format, title, diffms);// TODO newline?
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
    logMutex.lock();
    std::ofstream myfile;
#ifdef IS_TESTPY
    myfile.open("/tmp/eyemonpy.log", std::fstream::app);
#else
    myfile.open("/tmp/testlog.txt", std::fstream::app);
#endif
    //myfile << "Writing this to a file.\n";
    //myfile << str << std::endl ;
    myfile << str;
    myfile.close();// does at destruction
    //myfile.flush();
    //std::cout << str << std::endl ;
    logMutex.unlock();
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
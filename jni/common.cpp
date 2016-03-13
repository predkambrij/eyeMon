
#include <string.h>
#include <stdio.h>
#include <chrono>
#include <list>

#include <opencv2/objdetect/objdetect.hpp>

#include <common.hpp>

#ifdef IS_PHONE
#include <android/log.h>
#endif

#ifdef IS_PHONE
JNIEnv* env;
#endif

bool debug_print_when_queue_full = false;
bool debug_show_img = true;
bool debug_show_img_face = true;
bool debug_show_img_optfl_eyes = true;

//int method = METHOD_OPTFLOW;
int method = METHOD_TEMPLATE_BASED;

// optical flow
int flg=0;
///

std::list<BlinkMeasure> blinkMeasure;
std::list<BlinkMeasure> blinkMeasureShort;
int pause = 0;

int PHONE = 1;
std::chrono::high_resolution_clock::time_point startx = std::chrono::high_resolution_clock::now();

BlinkMeasure::BlinkMeasure(double timestamp, double lcor, double rcor) {
    this->timestamp = timestamp;
    this->lcor = lcor;
    this->rcor = rcor;
};

void measureBlinks() {
    long unsigned int blinkMeasureSize = blinkMeasure.size();
    if (blinkMeasureSize == 0) {
        return;
    }

    BlinkMeasure bm = blinkMeasure.front();
    blinkMeasureShort.push_back(bm);
    while (true) {
        BlinkMeasure oldestBm = blinkMeasureShort.front();
        if (oldestBm.timestamp < (bm.timestamp - 30000)) {
            blinkMeasureShort.pop_front();
        } else {
            break;
        }
    }

    int shortBmSize = blinkMeasureShort.size();
    if (shortBmSize < 100) {
        return;
    }

    double lavg = 0;
    double ravg = 0;
    std::list<BlinkMeasure>::iterator iter = blinkMeasureShort.begin();
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        lavg += bm.lcor;
        ravg += bm.rcor;
        iter++;
    }
    lavg = lavg/shortBmSize;
    ravg = ravg/shortBmSize;
    printf("sbm lavg:%.2lf ravg:%.2lf\n", lavg, ravg);
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
void difftime(char const *title, std::chrono::time_point<std::chrono::steady_clock> t1) {
    std::chrono::time_point<std::chrono::steady_clock> t2 = std::chrono::steady_clock::now();
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

    doLogClock1("%s: %ld", title, duration.count());
}

void doLogClock(const char* format, const char* title, double diffms) {
    if (PHONE != 1) {
        printf(format, title, diffms);
        printf("\n");
        return;
    }
}

void doLogClock1(const char* format, const char* title, long int diffms) {
    if (PHONE != 1) {
        printf(format, title, diffms);
        printf("\n");
        return;
    }
}

void doLog(const char* text) {
    if (PHONE != 1) {
        printf("%s\n", text);
        return;
    }
#ifdef IS_PHONE
    
    // malloc room for the resulting string
    char *szResult;
    szResult = (char*)malloc(sizeof(char)*500);

    auto end = std::chrono::high_resolution_clock::now();
    unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-startx).count();

    // standard sprintf
    sprintf(szResult, "res: %llu %s", ns, text);

    __android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, "NDK:OPTFLOF: [%s]", szResult);

    // cleanup
    free(szResult);
    
#endif
}

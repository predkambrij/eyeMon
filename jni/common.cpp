
#include <string.h>
#include <stdio.h>
#include <chrono>

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

int PHONE = 1;
std::chrono::high_resolution_clock::time_point startx = std::chrono::high_resolution_clock::now();

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

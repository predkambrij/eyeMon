
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

int PHONE = 1;
std::chrono::high_resolution_clock::time_point startx = std::chrono::high_resolution_clock::now();

void diffclock(char const *title, clock_t clock2) {
    clock_t clock1 = clock();
    double diffticks = 1000.0 * (clock1 - clock2);
    double diffms    = diffticks / CLOCKS_PER_SEC;

    doLogClock("%s: %f", title, diffms);
}

void doLogClock(const char* format, const char* title, double diffms) {
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

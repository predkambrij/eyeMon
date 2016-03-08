
#include <string.h>
#include <stdio.h>

#include <common.hpp>

int PHONE = 0;

void doLog(const char* text) {
    if (PHONE != 1) {
        printf("%s\n", text);
        return;
    }
    /*
    // malloc room for the resulting string
    char *szResult;
    szResult = (char*)malloc(sizeof(char)*500);

    auto end = std::chrono::high_resolution_clock::now();
    unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

    // standard sprintf
    sprintf(szResult, "res: %llu %s", ns, text);// szFormat sum CV_VERSION

    // get an object string
    jstring result = (env)->NewStringUTF(szResult);

    __android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, "NDK:OPTFLOF: [%s]", szResult);// szLogThis

    // cleanup
    free(szResult);*/
}

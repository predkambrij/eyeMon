#include <jni.h>

#include <string.h>
#include <stdio.h>
#include <android/log.h>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>
#include <chrono>

#include <farneback.hpp>

extern "C" {

long int grabberFrameNumF = 0;
Farneback farneb;

JNIEXPORT jlong JNICALL Java_org_blatnik_eyemon_Farneback_farnebackCreateObject
                                                                (JNIEnv * jenv, jclass, jstring jCascadeFileName) {
    const char *cascadeFileName = (jenv)->GetStringUTFChars(jCascadeFileName, 0);
    farneb.setup(cascadeFileName);
#ifdef IS_PHONE
    farneb.setJni(jenv);
#endif
    (jenv)->ReleaseStringUTFChars(jCascadeFileName, cascadeFileName);
    jlong result = 0;
    return result;
}

JNIEXPORT void JNICALL Java_org_blatnik_eyemon_Farneback_farnebackDestroyObject(JNIEnv * jenv, jclass, jlong thiz) {
}


JNIEXPORT void JNICALL Java_org_blatnik_eyemon_Farneback_farnebackDetect
                                                                (JNIEnv * jenv, jclass, jlong imageRGB, jlong imageGray) {
    cv::Mat rgb = *((cv::Mat*)imageRGB);
    cv::Mat gray = *((cv::Mat*)imageGray);

    long int tstp = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
    farneb.run(gray, rgb, tstp, grabberFrameNumF);
    grabberFrameNumF++;
}


} // end of extern C

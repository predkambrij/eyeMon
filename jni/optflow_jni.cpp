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

#include <optflow.hpp>

extern "C" {

long int grabberFrameNumO = 0;
OptFlow optf;

JNIEXPORT jlong JNICALL Java_org_blatnik_eyemon_OptFlow_optFlowCreateObject
                                                                (JNIEnv * jenv, jclass, jstring jCascadeFileName) {
    const char *cascadeFileName = (jenv)->GetStringUTFChars(jCascadeFileName, 0);
    optf.setup(cascadeFileName);
#ifdef IS_PHONE
    optf.setJni(jenv);
#endif
    (jenv)->ReleaseStringUTFChars(jCascadeFileName, cascadeFileName);
    jlong result = 0;
    return result;
}

JNIEXPORT void JNICALL Java_org_blatnik_eyemon_OptFlow_optFlowDestroyObject(JNIEnv * jenv, jclass, jlong thiz) {
}


JNIEXPORT void JNICALL Java_org_blatnik_eyemon_OptFlow_optFlowDetect
                                                                (JNIEnv * jenv, jclass, jlong imageRGB, jlong imageGray) {
    cv::Mat rgb = *((cv::Mat*)imageRGB);
    cv::Mat gray = *((cv::Mat*)imageGray);

    long int tstp = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
    optf.run(gray, rgb, tstp, grabberFrameNumO);
    grabberFrameNumO++;
}


} // end of extern C

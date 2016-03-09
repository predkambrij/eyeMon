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

#include <optflow.cpp>

using namespace cv;
using namespace std;

extern "C" {

OptFlow optf;

JNIEXPORT jlong JNICALL Java_org_opencv_samples_facedetect_OptFlow_optFlowCreateObject
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

JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_OptFlow_optFlowDestroyObject(JNIEnv * jenv, jclass, jlong thiz) {
}


JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_OptFlow_optFlowDetect
                                                                (JNIEnv * jenv, jclass, jlong imageRGB, jlong imageGray) {
    Mat rgb = *((Mat*)imageRGB);
    Mat gray = *((Mat*)imageGray);

    optf.run(rgb, gray);
}


} // end of extern C

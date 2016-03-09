#include <jni.h>

#include <opencv2/core/core.hpp>

#include <templatebased.cpp>

using namespace cv;
using namespace std;

extern "C" {

TemplateBased templBased;

JNIEXPORT jlong JNICALL Java_org_opencv_samples_facedetect_TemplateBasedJNI_templateBasedCreateObject
                                                                (JNIEnv * jenv, jclass, jstring jCascadeFileName) {
    const char *cascadeFileName = (jenv)->GetStringUTFChars(jCascadeFileName, 0);
    templBased.setup(cascadeFileName);
#ifdef IS_PHONE
    templBased.setJni(jenv);
#endif
    (jenv)->ReleaseStringUTFChars(jCascadeFileName, cascadeFileName);
    jlong result = 0;
    return result;
}

JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_TemplateBasedJNI_templateBasedDestroyObject(JNIEnv * jenv, jclass, jlong thiz) {
}


JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_TemplateBasedJNI_templateBasedDetect
                                                                (JNIEnv * jenv, jclass, jlong imageRGB, jlong imageGray) {
    Mat rgb = *((Mat*)imageRGB);
    Mat gray = *((Mat*)imageGray);

    templBased.run(gray, rgb);
}


} // end of extern C

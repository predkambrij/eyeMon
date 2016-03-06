package org.opencv.samples.facedetect;

import org.opencv.core.Mat;

public class TemplateBasedJNI {
    private long mNativeObj = 0;

    public TemplateBasedJNI(String cascadeName) {
        mNativeObj = templateBasedCreateObject(cascadeName);
    }

    public void detect(Mat imageRGB, Mat imageGray) {
        templateBasedDetect(imageRGB.getNativeObjAddr(), imageGray.getNativeObjAddr());
    }

    public void release() {
        templateBasedDestroyObject(mNativeObj);
        mNativeObj = 0;
    }

    private static native long templateBasedCreateObject(String cascadeName);
    private static native void templateBasedDetect(long inputImageRGB, long inputImageGray);
    private static native void templateBasedDestroyObject(long thiz);

}

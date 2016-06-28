package org.blatnik.eyemon;

import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;

public class Farneback {
    private long mNativeObj = 0;

    public Farneback(String cascadeName) {
        mNativeObj = farnebackCreateObject(cascadeName);
    }

    public void detect(Mat imageRGB, Mat imageGray) {
        farnebackDetect(imageRGB.getNativeObjAddr(), imageGray.getNativeObjAddr());
    }

    public void release() {
        farnebackDestroyObject(mNativeObj);
        mNativeObj = 0;
    }

    private static native long farnebackCreateObject(String cascadeName);
    private static native void farnebackDetect(long inputImageRGB, long inputImageGray);
    private static native void farnebackDestroyObject(long thiz);
}

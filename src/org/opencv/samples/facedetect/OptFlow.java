package org.opencv.samples.facedetect;

import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;

public class OptFlow {
    private long mNativeObj = 0;
    public OptFlow() {
        mNativeObj = optFlowCreateObject("cascadeName", 123);
    }

    public void detect(Mat imageRGB, Mat imageGray) {
        optFlowDetect(mNativeObj, imageRGB.getNativeObjAddr(), imageGray.getNativeObjAddr());
    }

    public void release() {
        optFlowDestroyObject(mNativeObj);
        mNativeObj = 0;
    }

    
    private static native long optFlowCreateObject(String cascadeName, int minFaceSize);
    private static native void optFlowDestroyObject(long thiz);
    private static native void optFlowDetect(long thiz, long inputImageRGB, long inputImageGray);
    
}

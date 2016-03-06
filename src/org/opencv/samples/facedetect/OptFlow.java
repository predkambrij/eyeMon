package org.opencv.samples.facedetect;

import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;

public class OptFlow {
    private long mNativeObj = 0;

    public OptFlow(String cascadeName) {
        mNativeObj = optFlowCreateObject(cascadeName);
    }

    public void detect(Mat imageRGB, Mat imageGray) {
        optFlowDetect(imageRGB.getNativeObjAddr(), imageGray.getNativeObjAddr());
    }

    public void release() {
        optFlowDestroyObject(mNativeObj);
        mNativeObj = 0;
    }

    private static native long optFlowCreateObject(String cascadeName);
    private static native void optFlowDetect(long inputImageRGB, long inputImageGray);
    private static native void optFlowDestroyObject(long thiz);
}

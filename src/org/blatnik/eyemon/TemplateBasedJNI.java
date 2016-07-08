package org.blatnik.eyemon;

import org.opencv.core.Mat;

import android.media.AudioManager;
import android.media.ToneGenerator;
import android.util.Log;

public class TemplateBasedJNI {
    private long mNativeObj = 0;

    public TemplateBasedJNI(String cascadeName) {
        mNativeObj = templateBasedCreateObject(cascadeName);
    }

    public void detect(Mat imageRGB, Mat imageGray) {
        int res = templateBasedDetect(imageRGB.getNativeObjAddr(), imageGray.getNativeObjAddr());
        if (res == 1) {
            ToneGenerator toneG = new ToneGenerator(AudioManager.STREAM_ALARM, 100);
            toneG.startTone(ToneGenerator.TONE_CDMA_ALERT_CALL_GUARD, 200);
        }
    }

    public void release() {
        templateBasedDestroyObject(mNativeObj);
        mNativeObj = 0;
    }

    private static native long templateBasedCreateObject(String cascadeName);
    private static native int templateBasedDetect(long inputImageRGB, long inputImageGray);
    private static native void templateBasedDestroyObject(long thiz);

}

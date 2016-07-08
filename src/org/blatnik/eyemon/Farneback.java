package org.blatnik.eyemon;

import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;

import android.media.AudioManager;
import android.media.ToneGenerator;

public class Farneback {
    private long mNativeObj = 0;

    public Farneback(String cascadeName) {
        mNativeObj = farnebackCreateObject(cascadeName);
    }

    public void detect(Mat imageRGB, Mat imageGray) {
        int res = farnebackDetect(imageRGB.getNativeObjAddr(), imageGray.getNativeObjAddr());
        if (res == 1) {
            ToneGenerator toneG = new ToneGenerator(AudioManager.STREAM_ALARM, 100);
            toneG.startTone(ToneGenerator.TONE_CDMA_ALERT_CALL_GUARD, 200);
        }
    }

    public void release() {
        farnebackDestroyObject(mNativeObj);
        mNativeObj = 0;
    }

    private static native long farnebackCreateObject(String cascadeName);
    private static native int farnebackDetect(long inputImageRGB, long inputImageGray);
    private static native void farnebackDestroyObject(long thiz);
}

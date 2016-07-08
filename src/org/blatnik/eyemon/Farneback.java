package org.blatnik.eyemon;

import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;

import android.media.AudioManager;
import android.media.ToneGenerator;

public class Farneback {
    private long mNativeObj = 0;
    private boolean debug_blink_beep = true;
    private boolean debug_n1_beep = true;
    private boolean debug_n2_beep = true;
    private boolean debug_n3_beep = true;

    private boolean n1state = false;
    private boolean n2state = false;
    private boolean n3state = false;

    public Farneback(String cascadeName) {
        mNativeObj = farnebackCreateObject(cascadeName);
    }

    public void detect(Mat imageRGB, Mat imageGray) {
        int res = farnebackDetect(imageRGB.getNativeObjAddr(), imageGray.getNativeObjAddr());
        boolean wasBlink = false;
        boolean activatedN1 = false;
        boolean activatedN2 = false;
        boolean activatedN3 = false;
        if (res % 2 == 1) {
            wasBlink = true;
        }
        res /= 2;
        if (res % 2 == 1) {
            activatedN1 = true;
        }
        res /= 2;
        if (res % 2 == 1) {
            activatedN2 = true;
        }
        res /= 2;
        if (res % 2 == 1) {
            activatedN3 = true;
        }

        if (wasBlink == true && this.debug_blink_beep == true) {
            ToneGenerator toneG = new ToneGenerator(AudioManager.STREAM_ALARM, 100);
            toneG.startTone(ToneGenerator.TONE_CDMA_ALERT_CALL_GUARD, 200);
        }
        if (activatedN1 == true) {
            if (this.n1state == false) {
                this.n1state = true;
                ToneGenerator toneG = new ToneGenerator(AudioManager.STREAM_ALARM, 100);
                toneG.startTone(ToneGenerator.TONE_CDMA_LOW_L, 10000);
            }
        } else {
            this.n1state = false;
        }
        if (activatedN2 == true) {
            if (this.n2state == false) {
                this.n2state = true;
                ToneGenerator toneG = new ToneGenerator(AudioManager.STREAM_ALARM, 100);
                toneG.startTone(ToneGenerator.TONE_CDMA_HIGH_SS, 10000);
            }
        } else {
            this.n2state = false;
        }
        if (activatedN3 == true) {
            if (this.n3state == false) {
                this.n3state = true;
                ToneGenerator toneG = new ToneGenerator(AudioManager.STREAM_ALARM, 100);
                toneG.startTone(ToneGenerator.TONE_CDMA_HIGH_SS, 10000);
            }
        } else {
            this.n3state = false;
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

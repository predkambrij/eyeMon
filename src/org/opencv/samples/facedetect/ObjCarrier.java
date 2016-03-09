package org.opencv.samples.facedetect;

import org.opencv.core.Mat;

public class ObjCarrier {
    public Mat gray;
    public long time;

    public ObjCarrier(Mat gray, long time) {
        this.gray = gray;
        this.time = time;
    }
}

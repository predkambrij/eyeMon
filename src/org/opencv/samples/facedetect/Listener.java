package org.opencv.samples.facedetect;

import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Mat;

import android.util.Log;

public class Listener  implements CvCameraViewListener2 {

    @Override
    public void onCameraViewStarted(int width, int height) {
        // TODO Auto-generated method stub
        
    }

    @Override
    public void onCameraViewStopped() {
        // TODO Auto-generated method stub
        
    }

    @Override
    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        Log.i("BEJE", "frame");
        return null;
    }

}

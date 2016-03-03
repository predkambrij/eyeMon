package org.opencv.samples.facedetect;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CameraPreview extends SurfaceView implements PreviewCallback {
    protected int mFrameWidth;
    protected int mFrameHeight;
    
    Camera mCamera;
    byte[] mBuffer;
    private static final String    TAG                 = "CameraPreview";
    SurfaceTexture mSurfaceTexture;
    private static final int MAGIC_TEXTURE_ID = 10;
    private Mat[] mFrameChain;
    private int mChainIdx = 0;
    private int testI = 0;
    
    private Thread mThread;

    public CameraPreview(Context context, int cameraId) {
        super(context);
//        super(context, cameraId);
    }

    @Override
    public void onPreviewFrame(byte[] frame, Camera arg1) {
        Log.i(TAG, "onPreviewFrame "+testI);
        Log.d(TAG, "Preview Frame received. Frame size: " + frame.length);
        
        FdActivity.buffer.add(frame);
        
//        try {
////            Mat m = new Mat(mFrameHeight + (mFrameHeight/2), mFrameWidth, CvType.CV_8UC1);
////            m.put(0, 0, frame);
//            FdActivity.buffer.add(frame);
//        } catch (Exception e) {
//            
//        }
        
        
        testI++;
        if (testI==300) {
            Mat m = new Mat(mFrameHeight + (mFrameHeight/2), mFrameWidth, CvType.CV_8UC1);
            m.put(0, 0, frame);
            Highgui.imwrite("/sdcard/fd/test.jpg", m);
        }
        
        
//        return mYuvFrameData.submat(0, mHeight, 0, mWidth);
        // Imgproc.cvtColor(mYuvFrameData, mRgba, Imgproc.COLOR_YUV2BGR_NV12, 4);
//        return mRgba;
//        public JavaCameraFrame(Mat Yuv420sp, int width, int height) {
//            super();
//            mWidth = width;
//            mHeight = height;
//            mYuvFrameData = Yuv420sp;
//            mRgba = new Mat();
//        }
//
//        public void release() {
//            mRgba.release();
//        }
//
//        private Mat mYuvFrameData;
//        private Mat mRgba;
//        private int mWidth;
//        private int mHeight;


        if (mCamera != null)
            mCamera.addCallbackBuffer(mBuffer);
        return;
    }

    public boolean connectCamera(int width, int height) {
        mFrameWidth = 640;
        mFrameHeight = 480;
        mCamera = Camera.open(1);
        Camera.Parameters params = mCamera.getParameters();
        params.setPreviewFormat(ImageFormat.NV21);
        params.setPreviewSize(mFrameWidth, mFrameHeight);
        params.setRecordingHint(true);
        mCamera.setParameters(params);
        params = mCamera.getParameters();
        int size = mFrameWidth * mFrameHeight;
        size  = size * ImageFormat.getBitsPerPixel(params.getPreviewFormat()) / 8;
        
        mBuffer = new byte[size];
        mCamera.addCallbackBuffer(mBuffer);
        mCamera.setPreviewCallbackWithBuffer(this);
        
        
        mSurfaceTexture = new SurfaceTexture(MAGIC_TEXTURE_ID);
        try {
            mCamera.setPreviewTexture(mSurfaceTexture);
        } catch (IOException e) {
            e.printStackTrace();
        }
        mCamera.startPreview();

        return true;
    }

    public void disconnectCamera() {
        mCamera.stopPreview();
        mCamera.release();
        return;
    }

//    private class FrameGrabber implements Runnable {
//
//        public void run() {
//            do {
//              Mat m = new Mat(mFrameHeight + (mFrameHeight/2), mFrameWidth, CvType.CV_8UC1);
//              m.put(0, 0, frame);
//              buffer.add(m);
//
//                FdActivity.buffer.add(null);
//                
//            } while (!mStopThread);
//            Log.d(TAG, "Finish processing thread");
//        }
//    }
}

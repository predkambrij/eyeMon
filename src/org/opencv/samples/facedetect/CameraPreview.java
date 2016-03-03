package org.opencv.samples.facedetect;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CameraPreview extends CameraBridgeViewBase implements PreviewCallback {
    
    Camera mCamera;
    byte[] mBuffer;
    private static final String    TAG                 = "CameraPreview";
    SurfaceTexture mSurfaceTexture;
    private static final int MAGIC_TEXTURE_ID = 10;
    private Mat[] mFrameChain;
    LinkedList<Mat> buffer = new LinkedList<Mat>();
    private int mChainIdx = 0;
    
    private Thread mThread;

    public CameraPreview(Context context, int cameraId) {
        super(context, cameraId);
    }

    @Override
    public void onPreviewFrame(byte[] frame, Camera arg1) {
        Log.i(TAG, "onPreviewFrame");
        Log.d(TAG, "Preview Frame received. Frame size: " + frame.length);
        
        Mat m = new Mat(mFrameHeight + (mFrameHeight/2), mFrameWidth, CvType.CV_8UC1);
        m.put(0, 0, frame);
        buffer.add(m);
        
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

    @Override
    protected boolean connectCamera(int width, int height) {
        mCamera = Camera.open(0);
        Camera.Parameters params = mCamera.getParameters();
        params.setPreviewFormat(ImageFormat.NV21);
        params.setPreviewSize(640, 480);
        params.setRecordingHint(true);
        mCamera.setParameters(params);
        params = mCamera.getParameters();
        mFrameWidth = 640;
        mFrameHeight = 480;
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

    @Override
    protected void disconnectCamera() {
        mCamera.stopPreview();
        mCamera.release();
        return;
    }
}

package org.opencv.samples.facedetect;

import java.io.IOException;
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
    private int mChainIdx = 0;
    
    private Thread mThread;

    public CameraPreview(Context context, int cameraId) {
        super(context, cameraId);
    }

    @Override
    public void onPreviewFrame(byte[] frame, Camera arg1) {
        Log.i(TAG, "onPreviewFrame");
        Log.d(TAG, "Preview Frame received. Frame size: " + frame.length);
        synchronized (this) {
            mFrameChain[1 - mChainIdx].put(0, 0, frame);
            this.notify();
        }
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
        
        mFrameChain = new Mat[2];
        mFrameChain[0] = new Mat(mFrameHeight + (mFrameHeight/2), mFrameWidth, CvType.CV_8UC1);
        mFrameChain[1] = new Mat(mFrameHeight + (mFrameHeight/2), mFrameWidth, CvType.CV_8UC1);
        AllocateCache();
        mCameraFrame = new JavaCameraFrame[2];
        mCameraFrame[0] = new JavaCameraFrame(mFrameChain[0], mFrameWidth, mFrameHeight);
        mCameraFrame[1] = new JavaCameraFrame(mFrameChain[1], mFrameWidth, mFrameHeight);
        
        
        mSurfaceTexture = new SurfaceTexture(MAGIC_TEXTURE_ID);
        try {
            mCamera.setPreviewTexture(mSurfaceTexture);
        } catch (IOException e) {
            e.printStackTrace();
        }
        mCamera.startPreview();
        
        mThread = new Thread(new CameraWorker());
        mThread.start();
        
//        mCamera.unlock();


//        List<Camera.Size> sizes = params.getSupportedPreviewSizes();
        return true;
    }

    @Override
    protected void disconnectCamera() {
//        mCamera.lock();
        mCamera.stopPreview();
        mCamera.release();
        return;
    }
    
    private boolean mStopThread;
    protected JavaCameraFrame[] mCameraFrame;
    private class CameraWorker implements Runnable {

        public void run() {
            do {
                synchronized (CameraPreview.this) {
                    try {
                        CameraPreview.this.wait();
                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                }

                if (!mStopThread) {
                    if (!mFrameChain[mChainIdx].empty())
                        deliverAndDrawFrame(mCameraFrame[mChainIdx]);
                    mChainIdx = 1 - mChainIdx;
                }
            } while (!mStopThread);
            Log.d(TAG, "Finish processing thread");
        }
    }
    
    private class JavaCameraFrame implements CvCameraViewFrame {
        public Mat gray() {
            return mYuvFrameData.submat(0, mHeight, 0, mWidth);
        }

        public Mat rgba() {
            Imgproc.cvtColor(mYuvFrameData, mRgba, Imgproc.COLOR_YUV2BGR_NV12, 4);
            return mRgba;
        }

        public JavaCameraFrame(Mat Yuv420sp, int width, int height) {
            super();
            mWidth = width;
            mHeight = height;
            mYuvFrameData = Yuv420sp;
            mRgba = new Mat();
        }

        public void release() {
            mRgba.release();
        }

        private Mat mYuvFrameData;
        private Mat mRgba;
        private int mWidth;
        private int mHeight;
    };
    

}

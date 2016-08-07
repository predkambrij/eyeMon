package org.blatnik.eyemon;

import java.io.IOException;
import java.util.List;

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.highgui.Highgui;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.util.Log;
import android.view.SurfaceView;

public class CameraPreview extends SurfaceView implements PreviewCallback {
    private int mFrameWidth;
    private int mFrameHeight;
    private SurfaceTexture mSurfaceTexture;
    private static final int MAGIC_TEXTURE_ID = 10;
    private byte[] mBuffer;
    private Camera mCamera;
    private static final String TAG = "CameraPreview";
//    private static int frameMaxSize = 30*60*10;
    private static int frameMaxSize = 600;

    public CameraPreview(Context context) {
        super(context);
    }

    @Override
    public void onPreviewFrame(byte[] frame, Camera arg1) {
        if (MainService.frameAdding == true) {
            if (MainService.frameList.size() < CameraPreview.frameMaxSize) {
                Mat gray = new Mat(MainService.widthHeight[1], MainService.widthHeight[0], CvType.CV_8UC1);
                gray.put(0, 0, frame);
                MainService.frameList.add(new ObjCarrier(gray, System.nanoTime()));

//                byte[] frameCopy = (byte[])frame.clone();
//                byte[] frameCopy = frame;
//                MainService.frameList.add(frameCopy);
//                MainService.frameList.add(gray);
//                MainService.frameTime.add(System.nanoTime());
                Log.i(TAG, "LENGTH "+MainService.frameList.size());
            } else {
                MainService.frameAdding = false;
            }
        }

        if (mCamera != null)
            mCamera.addCallbackBuffer(mBuffer);
        return;
    }

    public boolean connectCamera(int[] widthHeight, int camera) {
        mFrameWidth = widthHeight[0];
        mFrameHeight = widthHeight[1];
        mCamera = Camera.open(camera);
        Camera.Parameters params = mCamera.getParameters();
        List<int[]> fpss = params.getSupportedPreviewFpsRange();
        /*
        for(int[] fps : fpss) {
            String f = "";
            for (int ff :fps) {
                f+=ff+" ";
            }
            Log.i("FPSSS", ""+f);
        }*/
        params.setPreviewFpsRange(fpss.get(fpss.size()-1)[0], fpss.get(fpss.size()-1)[1]);
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
        mCamera.lock();
        mCamera.startPreview();

        return true;
    }

    public void disconnectCamera() {
        mCamera.stopPreview();
        mCamera.unlock();
        mCamera.release();
        return;
    }
}

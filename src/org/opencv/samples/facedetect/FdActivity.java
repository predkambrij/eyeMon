package org.opencv.samples.facedetect;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.LinkedList;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Scalar;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.objdetect.CascadeClassifier;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;

public class FdActivity extends Activity {

    private static final String TAG = "OCVSample::Activity";

    private MenuItem               startService;
    private MenuItem               stopService;

    private Mat                    mRgba;
    private Mat                    mGray;

    private CameraPreview mOpenCvCameraView;

    // native detector
    private OptFlow optFlow;
    
    public static volatile LinkedList<byte[]> buffer = new LinkedList<byte[]>();
    
    public FdActivity() {
    }

    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("detection_based_tracker");

                    try {
                        // load cascade file from application resources
                        InputStream is = getResources().openRawResource(R.raw.lbpcascade_frontalface);
                        File cascadeDir = getDir("cascade", Context.MODE_PRIVATE);
                        File mCascadeFile = new File(cascadeDir, "lbpcascade_frontalface.xml");
                        FileOutputStream os = new FileOutputStream(mCascadeFile);

                        byte[] buffer = new byte[4096];
                        int bytesRead;
                        while ((bytesRead = is.read(buffer)) != -1) {
                            os.write(buffer, 0, bytesRead);
                        }
                        is.close();
                        os.close();

                        // native library wrapper
                        optFlow = new OptFlow(mCascadeFile.getAbsolutePath());
                        cascadeDir.delete();
                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.e(TAG, "Failed to load cascade. Exception thrown: " + e);
                    }

                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };


    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.face_detect_surface_view);

        mOpenCvCameraView = new CameraPreview(this, 0);
        mOpenCvCameraView.connectCamera(640, 480);
        mOpenCvCameraView.setVisibility(1);
    }

    public void onPause()
    {
        super.onPause();
    }

    public void onResume()
    {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
    }

    public void onDestroy() {
        super.onDestroy();
        mOpenCvCameraView.disconnectCamera();
    }



//    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
//        mRgba = inputFrame.rgba();
//        // 1 means flip over y-axis
//        Core.flip(mRgba, mRgba, 1);
//        mGray = inputFrame.gray();
//
//        Core.putText(mRgba, " 3",
//                new org.opencv.core.Point(10,60), Core.FONT_HERSHEY_PLAIN, 3.0, new Scalar(0, 0, 0, 255));
//        
//        optFlow.detect(mRgba, mGray);
//        Log.i(TAG, "frame");
//        return mRgba;
//    }

    public boolean onCreateOptionsMenu(Menu menu) {
        startService = menu.add("startService");
        stopService  = menu.add("stopService");
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        if (item == startService) {
            
        } else if (item == stopService) {
            
        }

        return true;
    }
}

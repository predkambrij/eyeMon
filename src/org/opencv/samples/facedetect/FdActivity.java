package org.opencv.samples.facedetect;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;

public class FdActivity extends Activity {

    private static final String TAG = "OCVSample::Activity";

    private MenuItem startService;
    private MenuItem stopService;

    // native detector
    private OptFlow optFlow;
    public static List<byte[]> frameList = Collections.synchronizedList(new LinkedList<byte[]>());

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
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.face_detect_surface_view);
        
        Button startButton = (Button) findViewById(R.id.cameraRecorderStartService);
        startButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.i(TAG, "Staaart");
                    Intent intent = new Intent(FdActivity.this, DummyService.class);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startService(intent);
                }
            }
        );
        Button stopButton = (Button) findViewById(R.id.cameraRecorderStopService);
        stopButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.i(TAG, "Stooop");
                    stopService(new Intent(FdActivity.this, DummyService.class));
                }
            }
        );

    }

    public void onPause() {
        super.onPause();
    }

    public void onResume() {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
    }

    public void onDestroy() {
        super.onDestroy();
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
            Intent intent = new Intent(FdActivity.this, DummyService.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startService(intent);
        } else if (item == stopService) {
            stopService(new Intent(FdActivity.this, DummyService.class));
        }

        return true;
    }
}

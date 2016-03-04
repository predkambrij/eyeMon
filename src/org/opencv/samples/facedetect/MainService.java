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

import org.opencv.samples.facedetect.R;

import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class MainService extends Service {
    private static final String TAG = "MainService";
    private CameraPreview mOpenCvCameraView;
    private OptFlow optFlow;
    public static List<byte[]> frameList = Collections.synchronizedList(new LinkedList<byte[]>());

    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("eyemon");

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

    @Override
    public void onCreate() {
        mOpenCvCameraView = new CameraPreview(this, 0);
        mOpenCvCameraView.connectCamera(640, 480);
        mOpenCvCameraView.setVisibility(1);

           // Start foreground service to avoid unexpected kill
        Notification.Builder notificationB = new Notification.Builder(this)
            .setContentTitle("Background Video Recorder")
            .setContentText("")
            .setSmallIcon(R.drawable.icon);
        Notification notification = notificationB.getNotification();
        startForeground(1234, notification);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mOpenCvCameraView.disconnectCamera();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (!OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback)) {
            Log.i(TAG, "OpenCV Load Failure");
        } else {
            Log.i(TAG, "OpenCV Load success");
        }

        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

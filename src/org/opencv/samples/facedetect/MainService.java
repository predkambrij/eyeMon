package org.opencv.samples.facedetect;

import java.util.Calendar;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.highgui.Highgui;
import org.opencv.highgui.VideoCapture;

//import com.ImageInsightInc.GammaPixServiceTest.R;
import org.opencv.samples.facedetect.R;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.AlarmManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

public class MainService extends Service
{
    private static final String TAG = "Dummy service";
    private CameraPreview mOpenCvCameraView;

    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully in Dummy service");
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    @Override
    public void onCreate() 
    {   
        Log("DummyService: onCreate");
        
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
    public void onDestroy()
    {
        super.onDestroy();
        mOpenCvCameraView.disconnectCamera();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)  {
        if (!OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback)) {
            Log("OpenCV Load Failure");
        } else {
            Log("OpenCV Load success");
        }

        return START_NOT_STICKY;
    }

    public static void Log(String info)
    {
        android.util.Log.i("Dummy", info);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}

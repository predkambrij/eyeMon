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
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;
import org.opencv.samples.facedetect.R;

import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.Camera;
import android.os.IBinder;
import android.util.Log;

public class MainService extends Service {
    private static final String TAG = "MainService";
    private CameraPreview mOpenCvCameraView;
    private OptFlow optFlow;
    public static List<byte[]> frameList = Collections.synchronizedList(new LinkedList<byte[]>());
    public static volatile boolean frameAdding = true;
    protected int[] widthHeight;
    private Thread frameProcessor;
    private boolean frameProcessorRunning = true;

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
        this.frameProcessor = new Thread(new FrameProcessor());
        this.frameProcessor.start();

        this.mOpenCvCameraView = new CameraPreview(this);
//        this.widthHeight = new int[]{352, 288};
//        this.widthHeight = new int[]{1280, 720};
        this.widthHeight =  new int[]{640, 480};
        this.mOpenCvCameraView.connectCamera(this.widthHeight, 0);
        this.mOpenCvCameraView.setVisibility(1);

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
        this.frameProcessorRunning = false;
        this.mOpenCvCameraView.disconnectCamera();
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

    private class FrameProcessor implements Runnable {
        static final int METHOD_TEMPLATE = 0;
        static final int METHOD_OPTFLOW = 1;
        int method = 0;
        TemplateBased templateBased = null;

        public FrameProcessor() {
            this.method = 1;
            switch (method) {
            case METHOD_TEMPLATE:
                this.templateBased = new TemplateBased();
                this.templateBased.onCameraViewStarted();
                break;
            case METHOD_OPTFLOW:
                break;
            }
        }

        private void processFrame(byte[] frame) {
            Mat gray = new Mat(widthHeight[1] + (widthHeight[1]/2), widthHeight[0], CvType.CV_8UC1);
            gray.put(0, 0, frame);
            Mat rgb = new Mat();
            Imgproc.cvtColor(gray, rgb, Imgproc.COLOR_YUV2BGR_NV12, 4);
            Log.i(TAG, "I have it!");

            switch (method) {
            case METHOD_TEMPLATE:
                this.templateBased.onCameraFrame(rgb, gray);
                // cleanup
                this.templateBased.onCameraViewStopped();
                break;
            case METHOD_OPTFLOW:
                Highgui.imwrite("/sdcard/fd/test0.jpg", gray);
                optFlow.detect(gray, gray);
                Highgui.imwrite("/sdcard/fd/test1.jpg", gray);
                break;
            }
            rgb.release();
        }
        public void run() {
            while (frameProcessorRunning == true) {
                if (MainService.frameList.size() == 0) {
                    if (MainService.frameAdding == false) {
                        MainService.frameAdding = true;
                    }
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                    }
                } else {
                    byte[] frame = MainService.frameList.get(0);
                    this.processFrame(frame);
                }
            }
        }
    }
}

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
import android.widget.ImageView;

public class MainService extends Service {
    private static final String TAG = "MainService";
    private CameraPreview mOpenCvCameraView;
    private OptFlow optFlow;
    private TemplateBased templateBased;
    private TemplateBasedJNI templateBasedJni;
    public static List<byte[]> frameList = Collections.synchronizedList(new LinkedList<byte[]>());
    public static List<Long> frameTime = Collections.synchronizedList(new LinkedList<Long>());
    public static volatile boolean frameAdding = true;
    protected int[] widthHeight;
    private Thread frameProcessor;
    private boolean frameProcessorRunning = true;

    public static final String IMAGE_UPDATE        = "org.opencv.samples.facedetect.MainService.IMAGE_UPDATE";
    public static final String IMAGE_UPDATE_RESULT = "org.opencv.samples.facedetect.MainService.IMAGE_UPDATE_RESULT";

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
                        templateBased = new TemplateBased();
                        templateBased.onCameraViewStarted();
                        templateBasedJni = new TemplateBasedJNI(mCascadeFile.getAbsolutePath());

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
        this.mOpenCvCameraView.connectCamera(this.widthHeight, 1);
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
    
    private void sendBr(byte[] frame) {
        Intent intent = new Intent();
        intent.setAction(MainService.IMAGE_UPDATE_RESULT);
        intent.putExtra(MainService.IMAGE_UPDATE, "abc");
        this.sendBroadcast(intent);
    }

    private class FrameProcessor implements Runnable {
        static final int METHOD_OPTFLOW      = 0;
        static final int METHOD_TEMPLATE     = 1;
        static final int METHOD_TEMPLATE_JNI = 2;
        int method = 0;

        long frameCount      = 0;
        long timeStart       = 0;
        long frameCountMax   = 100;
        long totalTime       = 0;
        long grayConvertTime = 0;
        long releaseTime     = 0;
        long methodCallTime  = 0;

        int flSize = 0;

        public FrameProcessor() {
            switch (this.method) {
            case METHOD_OPTFLOW:
                break;
            case METHOD_TEMPLATE:
                break;
            case METHOD_TEMPLATE_JNI:
                break;
            }
        }

        private void processFrame(byte[] frame, long frameTime) {
            int  debug = 1;
            long start = System.nanoTime();
            Mat gray = new Mat(widthHeight[1] + (widthHeight[1]/2), widthHeight[0], CvType.CV_8UC1);
            gray.put(0, 0, frame);
            this.grayConvertTime += (System.nanoTime()-start);
//            long rgbStart = System.nanoTime();
//            Mat rgb = new Mat();
//            Imgproc.cvtColor(gray, rgb, Imgproc.COLOR_YUV2BGR_NV12, 4);
//            Log.i(TAG, String.format("processFrame rgb time: %d", (System.nanoTime()-rgbStart)/1000000));

            if (debug >= 2) {
                Highgui.imwrite("/sdcard/fd/gray_pre.jpg", gray);
            }
            long methodCall = System.nanoTime();
            switch (this.method) {
            // rgb gray
            case METHOD_OPTFLOW:
                optFlow.detect(gray, gray);
                break;
            case METHOD_TEMPLATE:
                templateBased.onCameraFrame(gray, gray);
                break;
            case METHOD_TEMPLATE_JNI:
                templateBasedJni.detect(gray, gray);
                break;
            }
            this.methodCallTime += (System.nanoTime()-methodCall);
            if (debug >= 1) {
                sendBr(frame);
            }
            if (debug >= 2) {
                Highgui.imwrite("/sdcard/fd/gray_post.jpg", gray);
            }

            long startRel = System.nanoTime();
//            rgb.release();
            gray.release();
            this.releaseTime += (System.nanoTime()-startRel);

            this.totalTime += (System.nanoTime()-start);
            this.frameCount++;
            if (this.frameCount == this.frameCountMax) {
                Log.i(TAG, "FL size: "+this.flSize);
                Log.i(TAG, String.format("avg frame capture rate %d", (frameTime-this.timeStart)/1000000/this.frameCountMax));
                Log.i(TAG, String.format("processFrame gray time: %d bytes %d", this.grayConvertTime/1000000/this.frameCountMax, frame.length));
                Log.i(TAG, String.format("processFrame methodCall time: %d", this.methodCallTime/1000000/this.frameCountMax));
                Log.i(TAG, String.format("processFrame release time: %d", this.releaseTime/1000000/this.frameCountMax));
                Log.i(TAG, String.format("processFrame time: %d", this.totalTime/1000000/this.frameCountMax));
                this.timeStart       = frameTime;
                this.frameCount      = 0;
                this.grayConvertTime = 0;
                this.methodCallTime  = 0;
                this.releaseTime     = 0;
                this.totalTime       = 0;
            }
        }

        private void cleanup() {
            switch (this.method) {
            case METHOD_OPTFLOW:
                optFlow.release();
                break;
            case METHOD_TEMPLATE:
                templateBased.onCameraViewStopped();
                break;
            case METHOD_TEMPLATE_JNI:
                templateBasedJni.release();
                break;
            }
        }

        public void run() {
            while (frameProcessorRunning == true) {
                this.flSize = MainService.frameList.size();
                if (this.flSize == 0) {
                    if (MainService.frameAdding == false) {
                        MainService.frameAdding = true;
                    }
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException e) {
                    }
                } else {
                    byte[] frame = MainService.frameList.remove(0);
                    long frameTime = MainService.frameTime.remove(0);
                    this.processFrame(frame, frameTime);
                }
            }
            this.cleanup();
        }
    }
}

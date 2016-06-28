package org.blatnik.eyemon;

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
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;
import org.blatnik.eyemon.R;

import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.hardware.Camera;
import android.os.IBinder;
import android.util.Log;
import android.widget.ImageView;

public class MainService extends Service {
    private static final String TAG = "MainService";
    private CameraPreview mOpenCvCameraView;
    private OptFlow optFlow;
    private Farneback farneback;
    private TemplateBased templateBased;
    private TemplateBasedJNI templateBasedJni;
//    public static List<Mat> frameList = Collections.synchronizedList(new LinkedList<Mat>());
//    public static List<byte[]> frameList = Collections.synchronizedList(new LinkedList<byte[]>());
    public static List<ObjCarrier> frameList = Collections.synchronizedList(new LinkedList<ObjCarrier>());
//    public static List<Long> frameTime = Collections.synchronizedList(new LinkedList<Long>());
    public static Bitmap bitmapImage;
    public static volatile boolean frameAdding = true;
//    public static int[] widthHeight =  new int[]{320, 240};
    public static int[] widthHeight =  new int[]{640, 480};
//    public static int[] widthHeight =  new int[]{352, 288};
//    public static int[] widthHeight =  new int[]{1280, 720};

    private Thread frameProcessor;
    private boolean frameProcessorRunning = true;

    public static final String IMAGE_UPDATE        = "org.blatnik.eyemon.MainService.IMAGE_UPDATE";
    public static final String IMAGE_UPDATE_RESULT = "org.blatnik.eyemon.MainService.IMAGE_UPDATE_RESULT";

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
                        MainService.this.optFlow = new OptFlow(mCascadeFile.getAbsolutePath());
                        MainService.this.farneback = new Farneback(mCascadeFile.getAbsolutePath());
                        MainService.this.templateBased = new TemplateBased();
                        MainService.this.templateBased.onCameraViewStarted();
                        MainService.this.templateBasedJni = new TemplateBasedJNI(mCascadeFile.getAbsolutePath());

                        MainService.this.frameProcessor = new Thread(new FrameProcessor());
                        MainService.this.frameProcessor.setPriority(Thread.MIN_PRIORITY);
                        MainService.this.frameProcessor.start();

                        MainService.this.mOpenCvCameraView = new CameraPreview(MainService.this);
                        MainService.this.mOpenCvCameraView.connectCamera(MainService.widthHeight, 1);
                        MainService.this.mOpenCvCameraView.setVisibility(1);

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
    
    private void sendBr(double lastFrameRate, Mat gray, Mat rgb, boolean debugInRGB) {
        Bitmap bmp = null;
        if (debugInRGB == true) {
            bmp = Bitmap.createBitmap(rgb.cols(), rgb.rows(), Bitmap.Config.ARGB_8888);
            Utils.matToBitmap(rgb, bmp);
        } else {
            bmp = Bitmap.createBitmap(gray.cols(), gray.rows(), Bitmap.Config.ARGB_8888);
            Utils.matToBitmap(gray, bmp);
        }
        MainService.bitmapImage = bmp;

        Intent intent = new Intent();
        intent.setAction(MainService.IMAGE_UPDATE_RESULT);
        intent.putExtra("lastFrameRate", lastFrameRate+"");
        this.sendBroadcast(intent);
    }

    private class FrameProcessor implements Runnable {
        static final int METHOD_OPTFLOW      = 0;
        static final int METHOD_TEMPLATE     = 1;
        static final int METHOD_TEMPLATE_JNI = 2;
        static final int METHOD_FARNEBACK = 3;
        int method = METHOD_FARNEBACK;

        long frameCount      = 0;
        long timeStart       = 0;
        long frameCountMax   = 100;
        long totalTime       = 0;
        long grayConvertTime = 0;
        long rgbConvertTime  = 0;
        long releaseTime     = 0;
        long methodCallTime  = 0;
        long garbageCollect  = 0;
        double lastFrameRate = 0;

        int flSize = 0;

        public FrameProcessor() {
            switch (this.method) {
            case METHOD_OPTFLOW:
                break;
            case METHOD_TEMPLATE:
                break;
            case METHOD_TEMPLATE_JNI:
                break;
            case METHOD_FARNEBACK:
                break;
            }
        }

//        private void processFrame(byte[] frame, long frameTime) {
        private void processFrame(Mat gray, long frameTime) {
            boolean debugInRGB = false;
            boolean debugWriteImages = false;
            boolean debugActivityUpdates = true;
//            Mat gray = null;
            Mat rgb = null;
            Mat tmp = null;
            long start = System.nanoTime();

//            gray = new Mat(widthHeight[1], widthHeight[0], CvType.CV_8UC1);
//            gray.put(0, 0, frame);
//            this.grayConvertTime += (System.nanoTime()-start);

            if (debugInRGB == true) {
//                long rgbStart = System.nanoTime();
//                tmp = new Mat(widthHeight[1] + (widthHeight[1]/2), widthHeight[0], CvType.CV_8UC1);
//                tmp.put(0, 0, frame);
//                rgb = new Mat();
//                Imgproc.cvtColor(tmp, rgb, Imgproc.COLOR_YUV2BGR_NV12, 4);
//                this.rgbConvertTime += System.nanoTime()-rgbStart;
            }

            if (debugWriteImages == true) {
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
            case METHOD_FARNEBACK:
                farneback.detect(gray, gray);
                break;
            }
            this.methodCallTime += (System.nanoTime()-methodCall);

            if (debugWriteImages == true) {
                Highgui.imwrite("/sdcard/fd/gray_post.jpg", gray);
            }

            if (debugActivityUpdates == true) {
                sendBr(this.lastFrameRate, gray, rgb, debugInRGB);
            }

            if (debugInRGB == true) {
                tmp.release();
                rgb.release();
            }

            long startRel = System.nanoTime();
            gray.release();
            this.releaseTime += (System.nanoTime()-startRel);

            this.totalTime += (System.nanoTime()-start);
            this.frameCount++;

//            long startGC = System.nanoTime();
//            System.gc();
//            this.garbageCollect += (System.nanoTime()-startGC);

            if (this.frameCount == this.frameCountMax) {
                double avgFrameTime = (((frameTime-this.timeStart)/(double)this.frameCountMax)/(double)1000000000);
                this.lastFrameRate = 1/avgFrameTime;
                Log.i(TAG, "FL size: "+this.flSize);
                Log.i(TAG, String.format("avg frame capture rate %.2f", this.lastFrameRate));
//                Log.i(TAG, String.format("processFrame gray time: %d bytes %d", this.grayConvertTime/1000000/this.frameCountMax, frame.length));
//                Log.i(TAG, String.format("processFrame rgb time: %d", this.rgbConvertTime/1000000/this.frameCountMax));
                Log.i(TAG, String.format("processFrame methodCall time: %d", this.methodCallTime/1000000/this.frameCountMax));
                Log.i(TAG, String.format("processFrame release time: %d", this.releaseTime/1000000/this.frameCountMax));
                Log.i(TAG, String.format("processFrame gc time: %d", this.garbageCollect/1000000/this.frameCountMax));
                Log.i(TAG, String.format("processFrame time: %d", this.totalTime/1000000/this.frameCountMax));
                this.timeStart = frameTime;
                this.frameCount = this.grayConvertTime = this.methodCallTime = this.releaseTime = this.totalTime = this.garbageCollect = 0;
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
            case METHOD_FARNEBACK:
                farneback.release();
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
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                    }
                } else {
//                    byte[] frame = MainService.frameList.remove(0);
//                    Mat frame = MainService.frameList.remove(0);
//                    long frameTime = MainService.frameTime.remove(0);
                    ObjCarrier frame = MainService.frameList.remove(0);
                    this.processFrame(frame.gray, frame.time);
                }
            }
            this.cleanup();
        }
    }
}

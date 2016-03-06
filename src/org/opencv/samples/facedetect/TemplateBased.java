package org.opencv.samples.facedetect;

import java.io.File;
import java.util.LinkedList;
import java.util.Queue;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Range;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.core.Core.MinMaxLocResult;
import org.opencv.imgproc.Imgproc;
import org.opencv.objdetect.CascadeClassifier;

import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;

public class TemplateBased {
    private static final String    TAG                 = "OCVSample::Activity";
    private static final Scalar    FACE_RECT_COLOR     = new Scalar(0, 255, 0, 255);
    private static final Scalar    EYE_RECT_COLOR     = new Scalar(255, 0, 0, 255);
    private static final Scalar    CUST_RECT_COLOR     = new Scalar(0, 0, 255, 255);
    private static final Scalar    CUST_RECT_L_COLOR     = new Scalar(0, 0, 127, 255);
    public static final int        JAVA_DETECTOR       = 0;
    public static final int        NATIVE_DETECTOR     = 1;

    private MenuItem               mItemTempl;
    
    private Mat openEyesTempl = null;
    private Mat templLeftEye = null;
    private Mat templRightEye = null;
    
    private boolean doItFlag = false;
    org.opencv.core.Point templLp1 = null;
    org.opencv.core.Point templLp2 = null;
    org.opencv.core.Point templRp1 = null;
    org.opencv.core.Point templRp2 = null;

    private Mat                    mRgba;
    private Mat                    mGray;
    private File[]                 mCascadeFiles = new File[2];
    private CascadeClassifier      mJavaDetector;
    private CascadeClassifier      mJavaDetectorEyes;

    private int                    mDetectorType       = JAVA_DETECTOR;
    private String[]               mDetectorName;

    private float                  mRelativeFaceSize   = 0.2f;
    private int                    mAbsoluteFaceSize   = 0;

    public TemplateBased() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    public void onCameraViewStarted() {
        mGray = new Mat();
        mRgba = new Mat();
    }

    public void onCameraViewStopped() {
        mGray.release();
        mRgba.release();
    }

    boolean initial = true;
    public Mat onCameraFrame(Mat mRgba, Mat mGray) {
//        if (initial == true) {
//            man_setup();
//            initial = false;
//        }
//        mRgba = inputFrame.rgba();
//        mGray = new Mat();
//        Imgproc.cvtColor(mRgba, mGray, Imgproc.COLOR_RGB2GRAY);
        
        //mGray = inputFrame.gray();
        mRgba = manipulation(mRgba, mGray);
        
        Core.putText(mRgba, "L:"+this.lBlinkCount+" R:"+this.rBlinkCount,
                //+String.format(" COR:%.2f", this.lCor),
                new org.opencv.core.Point(25,10), Core.FONT_HERSHEY_PLAIN, 1.0, new Scalar(0, 0, 0, 255));
        Core.putText(mRgba, String.format("LC:%.2f RC:%.2f", this.lCor, this.rCor),
                new org.opencv.core.Point(25,22), Core.FONT_HERSHEY_PLAIN, 1.0, new Scalar(0, 0, 0, 255));
        return mRgba;
    }
    
//    public void man_setup() {
//        String cascadeLoc = "/home/developer/opencv-2.4.10/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
//        mJavaDetectorEyes = new CascadeClassifier(cascadeLoc);
//        
//        if (mJavaDetectorEyes.empty()) {
//            System.out.println("Failed to load cascade classifier");
//        }
//        
//        return;
//    }
    
    //CascadeClassifier mJavaDetectorEyes;
    //float mAbsoluteFaceSize = 0.2f;
    //boolean doItFlag = true;
    
    public Mat manipulation(Mat mRgba, Mat mGray) {
        Mat rgb = new Mat();
        mRgba.copyTo(rgb);
        if (eyes_y == false) {
            
            MatOfRect eyes = new MatOfRect();
            
            mJavaDetectorEyes.detectMultiScale(mGray, eyes, 1.1, 2, 2, // TODO: objdetect.CV_HAAR_SCALE_IMAGE
                    new Size(mAbsoluteFaceSize, mAbsoluteFaceSize), new Size());
            
            
            int ret = this.cleanEyes_(eyes, mGray);
            System.out.println("ret: "+ret);
            if (ret == 0) {
                // nothing to do - eyes weren't found
            } else if(ret == 1) {
                // lost eyes
                this.lostEyes_(rgb);
            } else if(ret == 2) {
                //double templV[] = mGray.get(1, 1);
                //Log.i(TAG, "WTF eyes:"+templV);
                
                if (this.doItFlag == true) {
                    this.doItFlag = false;
                    
//                    this.openEyesTempl = mGray;
//                    this.templLp1 = this.lastLp1; this.templLp2 = this.lastLp2;
//                    this.templRp1 = this.lastRp1; this.templRp2 = this.lastRp2;
//                    this.extractEyes();
                                    
                }
                
                //this.countPixels(mRgba, lp1, lp2, rp1, rp2);
                //this.calculateCorrelations(mGray, mRgba);
                
                
                Core.rectangle(rgb, this.lastLp1, this.lastLp2, new Scalar(0, 0, 255, 255), 3);
                Core.rectangle(rgb, this.lastRp1, this.lastRp2, new Scalar(0, 0, 255, 255), 3);
                
    
            } else {
                System.out.println("nah...");
            }
        }
        if (eyes_y == true) {


            // / Create the result matrix
            int l_result_cols = rgb.cols() - l.cols() + 1;
            int l_result_rows = rgb.rows() - l.rows() + 1;
            
            Mat l_result = new Mat(l_result_rows, l_result_cols, CvType.CV_32FC1);
            
            // / Do the Matching and Normalize
            Imgproc.matchTemplate(mGray, l, l_result, Imgproc.TM_CCOEFF_NORMED);

            Core.MinMaxLocResult l_mmr = Core.minMaxLoc(l_result);
            
            Point l_matchLoc = l_mmr.maxLoc;
            System.out.println("max: "+l_mmr.maxVal+" min: "+l_mmr.minVal);
            
            Core.rectangle(rgb, l_matchLoc, new Point(l_matchLoc.x + l.cols(), l_matchLoc.y + l.rows()), new Scalar(0, 255, 0));
            
            System.out.println("L loc "+l_matchLoc.x+ " "+l_matchLoc.y);

            // / Create the result matrix
            int r_result_cols = rgb.cols() - r.cols() + 1;
            int r_result_rows = rgb.rows() - r.rows() + 1;
            
            Mat r_result = new Mat(r_result_rows, r_result_cols, CvType.CV_32FC1);
            
            // / Do the Matching and Normalize
            Imgproc.matchTemplate(mGray, r, r_result, Imgproc.TM_CCOEFF_NORMED);

            Core.MinMaxLocResult r_mmr = Core.minMaxLoc(r_result);
            
            Point r_matchLoc = r_mmr.maxLoc;
            System.out.println("max: "+r_mmr.maxVal+" min: "+r_mmr.minVal);
            this.lCor = l_mmr.maxVal;
            this.rCor = r_mmr.maxVal;
            
            Core.rectangle(rgb, r_matchLoc, new Point(r_matchLoc.x + l.cols(), r_matchLoc.y + l.rows()), new Scalar(0, 255, 0));
            
            System.out.println("L loc "+r_matchLoc.x+ " "+r_matchLoc.y);

        }
        
        return rgb;
    }
    
    private void lostEyes_(Mat mRgba) {
        if (lostTime == 0) {
            lostTime = System.currentTimeMillis();
        }
        if((System.currentTimeMillis()-lostTime) > 5000) { // blink larger than 5 seconds?
            // nothing, eyes are probably lost
        } else {
            // take last position
            //this.countPixels(mRgba, this.lastLp1, this.lastLp2,
            //        this.lastRp1, this.lastRp2);
            
            Core.rectangle(mRgba, this.lastLp1, this.lastLp2, new Scalar(0, 0, 127, 255), 3);
            Core.rectangle(mRgba, this.lastRp1, this.lastRp2, new Scalar(0, 0, 127, 255), 3);
            
        }
    }
    
    Mat l=null;// TODO new mat
    Mat r=null;
    boolean eyes_y = false;
    
    
    private int cleanEyes_(MatOfRect eyes, Mat mGray) {
        
        // return statuses
        // 0 nothing to do
        // 1 take lost eyes if any
        // 2 everything fine, for postprocessing
        
        Rect[] eyesArray = eyes.toArray();
        //for(int i=0; i<eyesArray.length;i++) {
        if (eyesArray.length == 2) {
            int left; int right;
            if (eyesArray[0].x < eyesArray[1].x) {
                // 0 is left, 1 is right
                left = 0; right = 1;
            } else {
                left = 1; right = 0;
            }
            
            // check that rectangles  are not overlapping, unless it's detected wrong
            if (eyesArray[right].x<(eyesArray[left].x+eyesArray[left].height)) {
                if (lostTime == 0 && this.lastLp1==null) {
                    // we have not position yet | fresh run
                    return 0;
                } else {
                    // eyes lost
                    return 1;
                }
            } else {
                l = mGray.submat(eyesArray[left]);
                r = mGray.submat(eyesArray[right]);
                //eyes_y = true;
                
                // looks that eyes were detected good
                org.opencv.core.Point lp1 = new org.opencv.core.Point(eyesArray[left].x, eyesArray[left].y);
                org.opencv.core.Point lp2 = new org.opencv.core.Point(
                                                                eyesArray[left].x+eyesArray[left].height,
                                                                eyesArray[left].y+eyesArray[left].width);
                org.opencv.core.Point rp1 = new org.opencv.core.Point(eyesArray[right].x, eyesArray[right].y);
                org.opencv.core.Point rp2 = new org.opencv.core.Point(
                                                                eyesArray[right].x+eyesArray[right].height, 
                                                                eyesArray[right].y+eyesArray[right].width);
                this.lastLp1 = lp1; this.lastLp2 = lp2;
                this.lastRp1 = rp1; this.lastRp2 = rp2;
                lostTime = 0;
                return 2;
            }
        } else {
            if (lostTime == 0 && this.lastLp1==null) {
                // we have not position yet | fresh run
                return 0;
            } else {
                // lost eyes
                return 1;
            }
        }
    }
    
        /*
//        
//        if (mAbsoluteFaceSize == 0) {
//            int height = mGray.rows();
//            if (Math.round(height * mRelativeFaceSize) > 0) {
//                mAbsoluteFaceSize = Math.round(height * mRelativeFaceSize);
//            }
//            for(int i=0; i<mNativeDetectors.length-1;i++){
//              mNativeDetectors[i].setMinFaceSize(mAbsoluteFaceSize);
//            }
//            
//        }

        //MatOfRect faces = new MatOfRect();
        MatOfRect eyes = new MatOfRect();
        
        //Mat eyeM1 = mGray.clone();
        //Mat eyeM2 = mGray.clone();
        // Mat eyesTholdM = mGray.clone();
        
        
        
        if (mDetectorType == JAVA_DETECTOR) {
            //if (mJavaDetector != null)
            //    mJavaDetector.detectMultiScale(mGray, faces, 1.1, 2, 2, // TODO: objdetect.CV_HAAR_SCALE_IMAGE
            //            new Size(mAbsoluteFaceSize, mAbsoluteFaceSize), new Size());
            
            // drops from 30 fps down to 1.8-2.25 when detected both eyes
            // acih! how to optimize that, acih!
            if (mJavaDetectorEyes != null)
                mJavaDetectorEyes.detectMultiScale(mGray, eyes, 1.1, 2, 2, // TODO: objdetect.CV_HAAR_SCALE_IMAGE
                        new Size(mAbsoluteFaceSize, mAbsoluteFaceSize), new Size());
            
            //openEyesTempl
            
            //Log.e(TAG, "Java tracker");
        }
        
//        else if (mDetectorType == NATIVE_DETECTOR) {
//          // TODO for(int i=0; i<mNativeDetectors.length;i++){
//            if (mNativeDetectors[0] != null)
//                mNativeDetectors[0].detect(mGray, faces);
//            Log.e(TAG, "Native tracker");
//          //throw new IllegalStateException("samo java, acih");
//        }
//        else {
//            Log.e(TAG, "Detection method is not selected!");
//        }
        
        // vecje -> ratuje vse temno
        // manjse -> ratuje bolj svetlo
        // 30 -> ve��er
        
        // ce se da sm pr uckih bi blo cool
        // TODO: count number of pixels, if an average is lower than X, increase thold value and vice versa
        //Imgproc.threshold(mGray, mRgba, this.thold_value, 255, Imgproc.THRESH_BINARY);
        
        
        //Rect[] facesArray = faces.toArray();
        //for (int i = 0; i < facesArray.length; i++)
        //    Core.rectangle(mRgba, facesArray[i].tl(), facesArray[i].br(), FACE_RECT_COLOR, 3);
        
        
        
        int ret = this.cleanEyes(eyes);
        if (ret == 0) {
            // nothing to do - eyes weren't found
        } else if(ret == 1) {
            // lost eyes
            this.lostEyes();
        } else if(ret == 2) {
            //double templV[] = mGray.get(1, 1);
            //Log.i(TAG, "WTF eyes:"+templV);
            if (this.doItFlag == true) {
                this.doItFlag = false;
                this.openEyesTempl = mGray;
                this.templLp1 = this.lastLp1; this.templLp2 = this.lastLp2;
                this.templRp1 = this.lastRp1; this.templRp2 = this.lastRp2;
                this.extractEyes();
            }
            
            //this.countPixels(mRgba, lp1, lp2, rp1, rp2);
            this.calculateCorrelations(mGray, mRgba);
            
            
            Core.rectangle(mRgba, this.lastLp1, this.lastLp2, CUST_RECT_COLOR, 3);
            Core.rectangle(mRgba, this.lastRp1, this.lastRp2, CUST_RECT_COLOR, 3);
            
        }
        
        Core.putText(mRgba, "L:"+this.lBlinkCount+" R:"+this.rBlinkCount,
                //+String.format(" COR:%.2f", this.lCor),
                new org.opencv.core.Point(25,10), Core.FONT_HERSHEY_PLAIN, 1.0, new Scalar(0, 0, 0, 255));
        Core.putText(mRgba, String.format("LC:%.2f RC:%.2f", this.lCor, this.rCor),
                new org.opencv.core.Point(25,22), Core.FONT_HERSHEY_PLAIN, 1.0, new Scalar(0, 0, 0, 255));
        return mRgba;
    */
    //}
    private void extractEyes() {
        int higher = 5;
        this.templLeftEye = new Mat(this.openEyesTempl, new Range(
                Math.max((int)this.templLp1.x-higher,0), Math.min((int)this.templLp2.x+higher,this.openEyesTempl.cols())
                                                ),
                
                                    new Range(
                Math.max((int)this.templLp1.y-higher,0), Math.min((int)this.templLp2.y+higher,this.openEyesTempl.rows())
                                            )
        );
        this.templRightEye = new Mat(this.openEyesTempl, new Range(
                Math.max((int)this.templRp1.x-higher,0), Math.min((int)this.templRp2.x+higher,this.openEyesTempl.cols())
                                                ),
                
                                    new Range(
                Math.max((int)this.templRp1.y-higher,0), Math.min((int)this.templRp2.y+higher,this.openEyesTempl.rows())
                                            )
        );
        return;
    }
    private void lostEyes() {
        if (lostTime == 0) {
            lostTime = System.currentTimeMillis();
        }
        if((System.currentTimeMillis()-lostTime) > 5000) { // blink larger than 5 seconds?
            // nothing, eyes are probably lost
        } else {
            // take last position
            //this.countPixels(mRgba, this.lastLp1, this.lastLp2,
            //        this.lastRp1, this.lastRp2);
            
            Core.rectangle(mRgba, this.lastLp1, this.lastLp2, CUST_RECT_L_COLOR, 3);
            Core.rectangle(mRgba, this.lastRp1, this.lastRp2, CUST_RECT_L_COLOR, 3);
            
        }
    }
    private void calculateCorrelations(Mat mGray, Mat mRgba) {
        // http://stackoverflow.com/questions/17001083/opencv-template-matching-example-in-android
        // http://docs.opencv.org/modules/imgproc/doc/object_detection.html?highlight=matchtemplate#matchtemplate
        
        // obsolete http://www.larmor.com/projects/JavaOpenCVMatchTemplate/doc/com/larmor/opencv/MatchTemplate.html
        // http://www.larmor.com/projects/JavaOpenCVMatchTemplate/doc/
        // http://www.larmor.com/portal/index.php?option=com_content&task=view&id=27&Itemid=60
        
        // scientific paper http://scribblethink.org/Work/nvisionInterface/nip.html
        // http://stackoverflow.com/questions/15215310/special-situation-in-normalized-cross-correlation-for-template-matching
        
        // http://docs.opencv.org/java/org/opencv/imgproc/Imgproc.html#matchTemplate(org.opencv.core.Mat, org.opencv.core.Mat, org.opencv.core.Mat, int)
        // http://en.wikipedia.org/wiki/Cross-correlation#Normalized_cross-correlation
        
        if (this.openEyesTempl == null) {
            // open eyes template isn't created yet
            return;
        }
        int match_method = Imgproc.TM_CCOEFF_NORMED;
        
        int result_cols = mRgba.cols() - templLeftEye.cols() + 1;
        int result_rows = mRgba.rows() - templLeftEye.rows() + 1;
        Mat result = new Mat(result_rows, result_cols, CvType.CV_32FC1);

        // / Do the Matching and Normalize
        Imgproc.matchTemplate(mGray, templLeftEye, result, match_method);
        
        // / Localizing the best match with minMaxLoc
        MinMaxLocResult mmr = Core.minMaxLoc(result);

        Point matchLoc;
        matchLoc = mmr.maxLoc;
        
        // / Show me what you got
        Core.rectangle(mRgba, matchLoc, new Point(matchLoc.x + templLeftEye.cols(),
                matchLoc.y + templLeftEye.rows()), new Scalar(0, 255, 0));
        this.lCor = mmr.maxVal;
        

        /*
        int lTempl_x_offset=0;
        int lEyes_x_offset=0;
        int lTempl_y_offset=0;
        int lEyes_y_offset=0;
        int lx_size = 0;
        int ly_size = 0;
        
        int rTempl_x_offset=0;
        int rEyes_x_offset=0;
        int rTempl_y_offset=0;
        int rEyes_y_offset=0;
        int rx_size = 0;
        int ry_size = 0;
        
        // find out which rectangular is wider (eyes or template)
        int lEyes_x_size = (int)(this.lastLp2.x-this.lastLp1.x);
        int lTempl_x_size = (int)(this.templLp2.x-this.templLp1.x);
        
        if ((lEyes_x_size) > (lTempl_x_size)) {
            lEyes_x_offset = (lEyes_x_size-lTempl_x_size)/2;
            lx_size = lTempl_x_size;
        } else {
            lTempl_x_offset = (lTempl_x_size-lEyes_x_size)/2;
            lx_size = lEyes_x_size;
        }
        int rEyes_x_size = (int)(this.lastRp2.x-this.lastRp1.x);
        int rTempl_x_size = (int)(this.templRp2.x-this.templRp1.x);
        
        if ((rEyes_x_size) > (rTempl_x_size)) {
            rEyes_x_offset = (rEyes_x_size-rTempl_x_size)/2;
            rx_size = rTempl_x_size;
        } else {
            rTempl_x_offset = (rTempl_x_size-rEyes_x_size)/2;
            rx_size = rEyes_x_size;
        }
        
        int lEyes_y_size = (int)(this.lastLp2.y-this.lastLp1.y);
        int lTempl_y_size = (int)(this.templLp2.y-this.templLp1.y);
        
        if ((lEyes_y_size) > (lTempl_y_size)) {
            lEyes_y_offset = (lEyes_y_size-lTempl_y_size)/2;
            ly_size = lTempl_y_size;
        } else {
            lTempl_y_offset = (lTempl_y_size-lEyes_y_size)/2;
            ly_size = lEyes_y_size;
        }
        int rEyes_y_size = (int)(this.lastLp2.y-this.lastLp1.y);
        int rTempl_y_size = (int)(this.templLp2.y-this.templLp1.y);
        
        if ((rEyes_y_size) > (rTempl_y_size)) {
            rEyes_y_offset = (rEyes_y_size-rTempl_y_size)/2;
            ry_size = rTempl_y_size;
        } else {
            rTempl_y_offset = (rTempl_y_size-rEyes_y_size)/2;
            ry_size = rEyes_y_size;
        }
        */
        
        
        /*
        double[][] lArrEye = new double[ly_size][lx_size]; // 1
        double[][] lArrTempl = new double[ly_size][lx_size]; // 2
        double[][] rArrEye = new double[ry_size][rx_size]; // 1
        double[][] rArrTempl = new double[ry_size][rx_size]; // 2
        
        for (int y=0;y<ly_size; y++) {
            int templ_y_i = y+lTempl_y_offset+(int)this.templLp1.y;
            int eyes_y_i = y+lEyes_y_offset+(int)this.lastLp1.y;
            
            for (int x=0; x<lx_size; x++) {
                int templ_x_i = x+lTempl_x_offset+(int)this.templLp1.x;
                int eyes_x_i = x+lEyes_x_offset+(int)this.lastLp1.x;
                //Log.i(TAG, "eyes:"+" y"+y+"x"+x+" Lp1x1"+(int)this.templLp1.x);
                //Log.i(TAG, "tmpl:"+" Lx1:"+(int)this.templLp1.x+" "
                //        +" Lx2:"+(int)this.templLp2.x+" "
                //        +" Ly1:"+(int)this.templLp1.y+" "
                //        +" Ly2:"+(int)this.templLp2.y+" "
                //        );
                //12-28 23:42:41.996:  eyes: Lp1x1:19  Lp1x2:45  Lp1y1:28  Lp1y2:54 

                double eyesV[] = eyes.get(eyes_y_i, eyes_x_i);
                double templV[] = this.openEyesTempl.get(templ_y_i, templ_x_i);
                //double templV[] = this.openEyesTempl.get(1, 1);
                
                //Log.i(TAG, "tmpl:"+" templ_y_i:"+templ_y_i+" "
                //        +" templ_x_i:"+templ_x_i
                //        +" len:"+templV.length
                //        );
                
                lArrEye[y][x] = eyesV[0];
                lArrTempl[y][x] = templV[0];
                
            }
        }
        for (int y=0;y<ry_size; y++) {
            int templ_y_i = y+rTempl_y_offset+(int)this.templLp1.y;
            int eyes_y_i = y+rEyes_y_offset+(int)this.lastLp1.y;
            
            for (int x=0; x<rx_size; x++) {
                int templ_x_i = x+rTempl_x_offset+(int)this.templLp1.x;
                int eyes_x_i = x+rEyes_x_offset+(int)this.lastLp1.x;
                //Log.i(TAG, "eyes:"+" y"+y+"x"+x+" Lp1x1"+(int)this.templLp1.x);
                //Log.i(TAG, "tmpl:"+" Lx1:"+(int)this.templLp1.x+" "
                //        +" Lx2:"+(int)this.templLp2.x+" "
                //        +" Ly1:"+(int)this.templLp1.y+" "
                //        +" Ly2:"+(int)this.templLp2.y+" "
                //        );
                //12-28 23:42:41.996:  eyes: Lp1x1:19  Lp1x2:45  Lp1y1:28  Lp1y2:54 

                double eyesV[] = eyes.get(eyes_y_i, eyes_x_i);
                double templV[] = this.openEyesTempl.get(templ_y_i, templ_x_i);
                //double templV[] = this.openEyesTempl.get(1, 1);
                
                //Log.i(TAG, "tmpl:"+" templ_y_i:"+templ_y_i+" "
                //        +" templ_x_i:"+templ_x_i
                //        +" len:"+templV.length
                //        );
                
                rArrEye[y][x] = eyesV[0];
                rArrTempl[y][x] = templV[0];
                
            }
        }
        //Log.i(TAG, "siz x:"+lx_size+" siz y:"+ly_size);
        this.lCor = this.correlationCoefficient(ly_size, lx_size, lArrEye, lArrTempl);
        this.rCor = this.correlationCoefficient(ry_size, rx_size, rArrEye, rArrTempl);
        
        //Correlation c = new Correlation();
        //c.calculateCorrelation(arr1, arr2);
        
        */
        
        
        return;
    }
    double lCor = 0;
    double rCor = 0;
    private double correlationCoefficient(
            int nrows, int ncols, double[][] arr1, double[][] arr2) {
        // Taken from http://www.cyut.edu.tw/~yltang/program/Correlation.java
        
        double  size = nrows * ncols;
        
        double   corr=0, mean1=0, mean2=0, mag1=0, mag2=0;
        // TODO optimize for template, that won't be calculated at each frame
        
        for (int i=0; i<nrows; i++) {   // Compute mean
            for (int j=0; j<ncols; j++) {
              mean1 += arr1[i][j];
              mean2 += arr2[i][j];
            }
          }
          mean1 /= size;
          mean2 /= size;
          // xx
          for (int i=0; i<nrows; i++) {
            for (int j=0; j<ncols; j++) {
              arr1[i][j] -= mean1;
              arr2[i][j] -= mean2;
              mag1 += arr1[i][j] * arr1[i][j];
              mag2 += arr2[i][j] * arr2[i][j];
              corr += arr1[i][j] * arr2[i][j];
              }
          }
          corr /= Math.sqrt(mag1*mag2);
          return corr;
    }
    private int cleanEyes(MatOfRect eyes) {
        // return statuses
        // 0 nothing to do
        // 1 take lost eyes if any
        // 2 everything fine, for postprocessing
        
        Rect[] eyesArray = eyes.toArray();
        //for(int i=0; i<eyesArray.length;i++) {
        if (eyesArray.length == 2) {
            int left; int right;
            if (eyesArray[0].x < eyesArray[1].x) {
                // 0 is left, 1 is right
                left = 0; right = 1;
            } else {
                left = 1; right = 0;
            }
            
            // check that rectangles  are not overlapping, unless it's detected wrong
            if (eyesArray[right].x<(eyesArray[left].x+eyesArray[left].height)) {
                if (lostTime == 0 && this.lastLp1==null) {
                    // we have not position yet | fresh run
                    return 0;
                } else {
                    // eyes lost
                    return 1;
                }
            } else {
                // looks that eyes were detected good
                org.opencv.core.Point lp1 = new org.opencv.core.Point(eyesArray[left].x, eyesArray[left].y);
                org.opencv.core.Point lp2 = new org.opencv.core.Point(
                                                                eyesArray[left].x+eyesArray[left].height,
                                                                eyesArray[left].y+eyesArray[left].width);
                org.opencv.core.Point rp1 = new org.opencv.core.Point(eyesArray[right].x, eyesArray[right].y);
                org.opencv.core.Point rp2 = new org.opencv.core.Point(
                                                                eyesArray[right].x+eyesArray[right].height, 
                                                                eyesArray[right].y+eyesArray[right].width);
                this.lastLp1 = lp1; this.lastLp2 = lp2;
                this.lastRp1 = rp1; this.lastRp2 = rp2;
                lostTime = 0;
                return 2;
            }
        } else {
            if (lostTime == 0 && this.lastLp1==null) {
                // we have not position yet | fresh run
                return 0;
            } else {
                // lost eyes
                return 1;
            }
        }
    }
    
    /**
     * timestamp, number of black pixels per eye
     */
    class QEntry {
        public int lBlacks;
        public int rBlacks;
        public int lWhites;
        public int rWhites;
        public long time;
        public QEntry(long time, int lBlacks, int rBlacks, int lWhites, int rWhites) {
            this.time = time;
            this.lBlacks=lBlacks; this.lWhites=lWhites;
            this.rBlacks=rBlacks; this.rWhites=rWhites;
        }
    }
    private long lastBlinkL=0; // non-0 blink didn't complete yet
    private long lastBlinkR=0; // non-0 blink didn't complete yet
    
    private int lBlinkCount=0; // non-0 blink didn't complete yet
    private int rBlinkCount=0; // non-0 blink didn't complete yet
    
    // variables
    private int thold_value_default = 30;
    private int thold_value = thold_value_default;
    private long firstTime = System.currentTimeMillis();
    private long lastUpdatedTV = 0;
    Queue<QEntry> queue = new LinkedList<QEntry>();
    Queue<String> lBlinkLog = new LinkedList<String>();
    Queue<String> rBlinkLog = new LinkedList<String>();
    
    
    
    // last position eyes
    org.opencv.core.Point lastLp1 = null;
    org.opencv.core.Point lastLp2 = null;
    org.opencv.core.Point lastRp1 = null;
    org.opencv.core.Point lastRp2 = null;
    private long lostTime = 0;//System.currentTimeMillis();
    
    private boolean countPixels(Mat mRgba, org.opencv.core.Point lp1, org.opencv.core.Point lp2,
                                           org.opencv.core.Point rp1, org.opencv.core.Point rp2) {
        int lBlacks=0;
        int rBlacks=0;
        int lWhites=0;
        int rWhites=0;
        
        for (int eye=0; eye<2; eye++) {
            // black pixels per vertical row
            // if there are two occurs of bigger number and in the middle almost zero,
            //    take just the second one because of the eyebrow
            org.opencv.core.Point p1 = null;
            org.opencv.core.Point p2 = null;
            if (eye == 0) {// left
                p1 = lp1; p2 = lp2;
            } else if (eye == 1) { // right
                p1 = rp1; p2 = rp2;
            }
            
            int ysize = (int)(p2.y-p1.y);
            int xsize = (int)(p2.x-p1.x);
            int[][] rows = new int[ysize][2];
            for (int i=0; i<ysize; i++) {
                // initialization
                rows[i][0]=0; rows[i][1]=0;
            }
            
            for(int y=0; y<ysize; y++) {
                for(int x=0; x<xsize; x++) {
                    double val[] = mRgba.get(y+(int)p1.y, x+(int)p1.x);
                    
                    if (val[0] == 0) {
                        // blacks
                        rows[y][0]+=1;
                    } else if(val[0] == 255) {
                        // whites
                        rows[y][1]+=1;
                    } else {
                        throw new IllegalStateException("AAACIIH!");
                    }
                    
                }
            }
            
            // left or right?
            int blackNwhite[] = this.calculatePixels(rows, ysize, eye);
            if (eye == 0) { // left
                lBlacks += blackNwhite[0];
                lWhites += blackNwhite[1];
            } else if (eye == 1) { // right
                rBlacks += blackNwhite[0];
                rWhites += blackNwhite[1];
            }
        }
        
        this.framePostProc(lBlacks, rBlacks, lWhites, rWhites);
        return false;
    }
    /**
     * calculate number of pixels (consider eyebrows)
     */
    private int[] calculatePixels(int[][] rows, int ysize, int eye) {
        
        String p = "";
        if (eye == 0) { // left
            p += "L ";
        } else if (eye == 1) { // right
            p += "R ";
        }
        for (int i=0; i<ysize; i++) {
            double ratio = (rows[i][0]+1)/((double)(rows[i][1]+1));
            p += String.format("%d,%d ; ", rows[i][0], rows[i][1]);
            
        }
        p+="            ";
        for (int i=0; i<ysize; i++) {
            double ratio = (rows[i][0]+1)/((double)(rows[i][1]+1));
            p += String.format("%.2f, ", ratio);
            
        }
        Log.i(TAG, String.format("%s",p));
        // nuke eyebrows ( always jump first threshold unless it's the only one )
        // 12-27 02:02:39.578: E/OCVSample::Activity(6225): 
        // R 2,53 ; 8,47 ; 17,38 ; 20,35 ; 22,33 ; 19,36 ; 10,45 ; 4,51 ; 2,53 ; <-- eyebrow
        // eye
        // 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 4,51 ; 11,44 ; 6,49 ; 2,53 ; 11,44 ; 
        // 14,41 ; 13,42 ; 14,41 ; 15,40 ; 15,40 ; 13,42 ; 11,44 ; 10,45 ; 8,47 ; 3,52 ; 4,51 ; 
        // 0,55 ; 3,52 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 
        // 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 0,55 ; 

        /*
         * 12-27 02:07:06.208: E/OCVSample::Activity(6943): // nothing special, eye
         * L 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 
         * 0.05, 0.15, 0.32, 0.18, 0.12, 0.12, 0.15, 0.15, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02,
         * 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 
        
         * 12-27 02:07:06.218: E/OCVSample::Activity(6943): 
         * eyebrows (lasts 7 entries)
         * R 0.02, 0.48, 0.59, 0.59, 0.31, 0.18, 0.05, 0.02, 0.02, 0.02, 0.02, (11)
         * eye
         * 0.12, 0.39, 0.39, 0.28, 0.44, 0.64, 0.77, 0.48, 0.53, 0.53, 0.64, 0.53, 0.48, 0.44, 0.39, (15)
         * 0.35, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, 0.02, (15)
         * 0.02, 0.02, 0.02, (3)
         * (total 44)

         */
        
        // check if there is breach of (let's say 0.3) in the first 0.40*ysize
        //                          - and it ends to 0.02 (then we say that's all because of the eyebrow)
        double eyebrow_high = 0.3;
        double eyebrow_low = 0.1;
        int eyebrow_highest_i = 0;
        int eyebrow_lowest_i = 0;
        int last_i=0;
        int no_count=0;
        boolean allow_highest = true;
        boolean allow_lowest = true;
        int eye_highest_i = 0;
        double eyebrow_lowest_after = 0;
        boolean eyebrow_high_ack = false;
        boolean eyebrow_low_ack = false;
        
        for (int i=0; i<ysize; i++) {
            if //(i<(ysize*0.40))
            (true)
            {
                double ratio = (rows[i][0]+1)/((double)(rows[i][1]+1));
                double ratio_highest = (rows[eyebrow_highest_i][0]+1)/((double)(rows[eyebrow_highest_i][1]+1));
                double ratio_lowest = (rows[eyebrow_lowest_i][0]+1)/((double)(rows[eyebrow_lowest_i][1]+1));
                double ratio_eye_highest = (rows[eye_highest_i][0]+1)/((double)(rows[eye_highest_i][1]+1));
                //p += String.format("%d,%d ; ", rows[i][0], rows[i][1]);
                //p += String.format("%.2f, ", ratio);
                
                if (allow_highest == true) {
                    if (ratio > ratio_highest) {
                        // remember the highest one
                        eyebrow_highest_i = i;
                        no_count=0;
                    } else {
                        no_count++;
                        if (no_count == 3) {
                            allow_highest=false;
                            eyebrow_lowest_i=i;
                        }
                    }
                } else {
                    if(allow_lowest == true) {
                        if (ratio < ratio_lowest) {
                            eyebrow_lowest_i=i;
                            no_count=0;
                        } else {
                            no_count++;
                            if (no_count == 3) {
                                allow_lowest=false;
                                eye_highest_i=i;
                            }
                        }
                    } else {
                        if (ratio > ratio_eye_highest) {
                            eye_highest_i=i;
                        }
                        // probably eye begins
                        // TODO note the highest one for the eye
                    }
                }
                
                
                /*if (eyebrow_high_ack == true) {
                    eyebrow_lowest_after = 
                }
                if (eyebrow_high_ack == true && ratio < eyebrow_low) {
                    eyebrow_low_ack=true;
                    break;
                }*/
            } else {
                
            }
        }
        double ratio_highest = (rows[eyebrow_highest_i][0]+1)/((double)(rows[eyebrow_highest_i][1]+1));
        double ratio_lowest = (rows[eyebrow_lowest_i][0]+1)/((double)(rows[eyebrow_lowest_i][1]+1));
        double ratio_eye_highest = (rows[eye_highest_i][0]+1)/((double)(rows[eye_highest_i][1]+1));
        
        Log.i(TAG, String.format("size:%d highest eyeb:%d lowest eyeb:%d highest eye:%d",ysize, eyebrow_highest_i,
                eyebrow_lowest_i, eye_highest_i));
        Log.i(TAG, String.format("size:%d highest eyeb:%.2f lowest eyeb:%.2f highest eye:%.2f",ysize, ratio_highest,
                ratio_lowest, ratio_eye_highest));
        
        if (ratio_lowest*2 < ratio_eye_highest) {
            // if eye is at least 2 times bigger than lowest part
            //    below of the eyebrow then we will erase eyebrow
            // unless we think that eyebrow isn't present
            
            for (int i=0; i<eyebrow_lowest_i; i++) {
                // turn everything into white
                rows[i][1]= (rows[i][1] + rows[i][0]);
                rows[i][0]=0;
            }
        }
        
        /*
         * the old way of erasing
        if (eyebrow_high_ack == true && eyebrow_low_ack==true) {
            for (int i=0; i<last_i; i++) {
                // turn everything into white
                rows[i][1]= (rows[i][1] + rows[i][0]);
                rows[i][0]=0;
            }
        }*/
        // add fuse that eyebrow cannot be bigger than 0.4*ysize
        
        int blacks = 0;
        int whites = 0;
        for (int i=0; i<ysize; i++) {
            whites += rows[i][1];
            blacks += rows[i][0];
        }
        return new int[]{blacks, whites};
    }
    
    /**
     * add entries to the queue of last X seconds and try to figure out blinks
     *      (timewindow) (because of differing light conditions)
     */
    private void framePostProc(int lBlacks, int rBlacks, int lWhites, int rWhites) {
        long entryTime = System.currentTimeMillis();
        QEntry frameData = new QEntry(entryTime, lBlacks, rBlacks, lWhites, rWhites);
        queue.add(frameData);
        
        // drop old frames
        int qFrame = 10000;
        while ((entryTime-(queue.peek().time)) > qFrame) { // X seconds frame
            queue.poll();
        }
        
        // let's look for time of the oldest element
        int aLBlacks=0, aLWhites=0;
        int aRBlacks=0, aRWhites=0;
        
        
        for(QEntry e:queue) {
            // calculate the average
            aLBlacks += e.lBlacks;
            aLWhites += e.lWhites;
            aRBlacks += e.rBlacks;
            aRWhites += e.rWhites;
        }
        double aaLBlacks=aLBlacks/((double)queue.size());
        double aaLWhites=aLWhites/((double)queue.size());
        double aaRBlacks=aRBlacks/((double)queue.size());
        double aaRWhites=aRWhites/((double)queue.size());
        //Log.e(TAG, String.format("aBlacks:%.2f aLWhites:%.2f size:%.2f",aaLBlacks,aaLWhites,(double)queue.size()));
        double avgL = (aaLBlacks+1)/((double)(aaLWhites+1));
        double curL= (lBlacks+1)/((double)(lWhites+1));
        double avgR = (aaRBlacks+1)/((double)(aaRWhites+1));
        double curR= (rBlacks+1)/((double)(rWhites+1));
        
        Log.e(TAG, String.format("L avg ratio:%.2f cur ratio:%.2f THOL:%d proc: %.2f",avgL,curL,this.thold_value,
                                                                                curL/avgL));
        Log.e(TAG, String.format("R avg ratio:%.2f cur ratio:%.2f         proc: %.2f",avgR,curR,curL/avgL));
        Log.e(TAG, String.format("xL avg blacks:%.2f cur blacks:%d proc: %.2f",aaLBlacks,lBlacks,((double)lBlacks)/aaLBlacks));
        Log.e(TAG, String.format("xR avg blacks:%.2f cur blacks:%d proc: %.2f",aaRBlacks,rBlacks,((double)rBlacks)/aaRBlacks));
        
        // if it falls bellow of 60%, it's blink
        double blink_threshold = 0.6;
        // left blink management
        if ((((double)lBlacks)/aaLBlacks) < blink_threshold) {
            if (this.lastBlinkL == 0) {
                this.lastBlinkL = frameData.time;
            } else {
                // blink continues
            }
        } else {
            if (this.lastBlinkL == 0) {
                // it's not blink at all
            } else {
                // blink just ended
                this.lBlinkLog.add("start:"+this.lastBlinkL+" end:"+frameData.time);
                this.lBlinkCount++;
                this.lastBlinkL=0;
            }
        }
        // right blink management
        if ((((double)rBlacks)/aaRBlacks) < blink_threshold) {
            if (this.lastBlinkR == 0) {
                this.lastBlinkR = frameData.time;
            } else {
                // blink continues
            }
        } else {
            if (this.lastBlinkR == 0) {
                // it's not blink at all
            } else {
                // blink just ended
                this.rBlinkLog.add("start:"+this.lastBlinkR+" end:"+frameData.time);
                this.rBlinkCount++;
                this.lastBlinkR=0;
            }
        }
        
        
        // update threshold if needed
        if (this.lostTime == 0) {
            if (avgL<0.1 && avgR<0.1) {
                if ((System.currentTimeMillis()-lastUpdatedTV) > qFrame) {
                    if (avgL<0.03 && avgR<0.03) {
                        this.thold_value+=4;
                    } else {
                        this.thold_value+=2;
                    }
                    
                    lastUpdatedTV = System.currentTimeMillis();
                }
            }
            if (avgL>0.5 || avgR>0.5) {
                if ((System.currentTimeMillis()-lastUpdatedTV) > qFrame) {
                    this.thold_value-=2;
                    lastUpdatedTV = System.currentTimeMillis();
                }
            }
            
        }
        return;
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "called onCreateOptionsMenu");
        mItemTempl   = menu.add("Open Eyes Templ");
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "called onOptionsItemSelected; selected item: " + item);
        if (item == mItemTempl) {
            //this.doItFlag = true;
            eyes_y = (!eyes_y);
        }
        return true;
    }


}

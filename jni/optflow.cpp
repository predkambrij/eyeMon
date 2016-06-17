
#ifdef IS_PHONE
#include <jni.h>
#include <android/log.h>
#endif

#include <string.h>
#include <stdio.h>

#include <iostream>
#include <ctype.h>
#include <chrono>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

//#include <eyeLike/src/main.cpp>
#include "eyeLike/src/findEyeCenter.h"


#include <common.hpp>
#include <optflow.hpp>

void drawOptFlowMap (cv::Rect face, cv::Rect eyeE, const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye) {
    //cv::circle(cflowmap, cv::Point2f((float)10, (float)10), 3, cv::Scalar(0,255,0), -1, 8);
    cv::circle(cflowmap, cv::Point2f((float)15, (float)15), 10, cv::Scalar(0,255,0), -1, 8);
    int xo, yo;
    // if (eye == 0) {
    //     xo = leftXOffset;
    //     yo = leftYOffset;
    // } else {
    //     xo = rightXOffset;
    //     yo = rightYOffset;
    // }
    xo = face.x+eyeE.x;
    yo = face.y+eyeE.y;
    for(int y = 0; y < flow.rows; y += step) {
        for(int x = 0; x < flow.cols; x += step) {
            
            const cv::Point2f& fxy = flow.at< cv::Point2f>(y, x);
            int px = x+xo, py = y+yo;
            cv::line(cflowmap, cv::Point(px,py), cv::Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), color);
            //circle(cflowmap, Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), 1, color, -1);
            cv::circle(cflowmap, cv::Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), 1, color, -1, 8);

            //circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
            printf("%.0f ", fxy.y);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

void getLeftRightEyeMat(cv::Mat gray, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Mat *left, cv::Mat *right) {
    // printf("eye_region_width %d, eye_region_height %d\n", eye_region_width, eye_region_height);
    // printf("leftEyeRegion %d, leftEyeRegion %d\n", leftEyeRegion.x, leftEyeRegion.y);
    // printf("rightEyeRegion %d, rightEyeRegion %d\n", rightEyeRegion.x, rightEyeRegion.y);
    // gray(cv::Rect(leftEyeRegion.x, leftEyeRegion.y, 60, 70)).copyTo(*left);
    // gray(cv::Rect(rightEyeRegion.x, rightEyeRegion.y, 60, 70)).copyTo(*right);
    gray(cv::Rect(leftXOffset, leftYOffset, leftCols, leftRows)).copyTo(*left);
    gray(cv::Rect(rightXOffset, rightYOffset, rightCols, rightRows)).copyTo(*right);
}

int faceDetect(cv::Mat gray, cv::Rect *face) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
    if (faces.size() != 1) {
        return -1;
    }
    *face = faces[0];
    return 0;
}
// void eyeRegions(cv::Rect face, cv::Rect *leftEyeRegion, cv::Rect *rightEyeRegion) {
//     eye_region_width = face.width * (kEyePercentWidth/100.0);
//     eye_region_height = face.width * (kEyePercentHeight/100.0);
//     int eye_region_top = face.height * (kEyePercentTop/100.0);
//     (*leftEyeRegion) = cv::Rect(face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
//     (*rightEyeRegion) = cv::Rect(face.width - eye_region_width - face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
// }
void eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point &leftPupil, cv::Point &rightPupil) {
    //leftPupil  = findEyeCenter(faceROI, leftEyeRegion);
    rightPupil = findEyeCenter(faceROI, rightEyeRegion);
}

void showResult(cv::Mat cflow, cv::Rect face, cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point leftPupil, cv::Point rightPupil) {
    // // change eye centers to face coordinates
    // rightPupil.x += rightEyeRegion.x; rightPupil.y += rightEyeRegion.y;
    // leftPupil.x += leftEyeRegion.x; leftPupil.y += leftEyeRegion.y;
    // rightPupil.x += face.x; rightPupil.y += face.y;
    // leftPupil.x += face.x; leftPupil.y += face.y;
    // // leftXOffset = leftPupil.x - 50; rightXOffset = rightPupil.x - 50;
    // // leftYOffset = leftPupil.y - 50; rightYOffset = rightPupil.y - 50;

    // // if (leftXavg==0) {
    // //     // first iteration
    // //     leftXp1=leftPupil.x; leftXlast=leftPupil.x; leftXavg=leftPupil.x;
    // //     leftYp1=leftPupil.y; leftYlast=leftPupil.y; leftYavg=leftPupil.y;
    // //     rightXp1=rightPupil.x; rightXlast=rightPupil.x; rightXavg=rightPupil.x;
    // //     rightYp1=rightPupil.y; rightYlast=rightPupil.y; rightYavg=rightPupil.y;
    // // } else {
    // //     // calculate avg
    // //     leftXavg=(leftXp1+leftXlast+leftPupil.x)/3; leftYavg=(leftYp1+leftYlast+leftPupil.y)/3;
    // //     rightXavg=(rightXp1+rightXlast+rightPupil.x)/3; rightYavg=(rightYp1+rightYlast+rightPupil.y)/3;
    // //     // rotate
    // //     leftXp1=leftXlast; rightXp1=rightXlast; leftYp1=leftYlast; rightYp1=rightYlast;
    // //     leftXlast=leftPupil.x; rightXlast=rightPupil.x; leftYlast=leftPupil.y; rightYlast=rightPupil.y;
    // // }
    // // leftXOffset = leftXavg-50; leftYOffset = leftYavg-50;
    // // rightXOffset = rightXavg-50; rightYOffset = rightYavg-50;

    // circle(cflow, Point2f((float)15, (float)15), 10, Scalar(0,255,0), -1, 8);
    // circle(cflow, rightPupil, 3, Scalar(0,255,0), -1, 8);
    // circle(cflow, leftPupil, 3, Scalar(0,255,0), -1, 8);

    // draw eye centers
    if (debug_show_img_main == true && PHONE == 0) {
        imshow("main", cflow);
    }
}

void OptFlow::process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    clock_t start;
    cv::Point leftPupil, rightPupil;
//    cv::Rect face, leftEyeRegion, rightEyeRegion;
    cv::Mat faceROI;
    cv::Mat left, right;
    cv::Mat flowLeft, flowRight;
/*
    start = clock();
    if (faceDetect(gray, &face) != 0) {
        if (debug_show_img_main == true && PHONE == 0) {
            imshow("main", out);
        }
        return;
    }
*/
    // // faceROI = gray(face);
    // if (kSmoothFaceImage) {
    //     double sigma = kSmoothFaceFactor * gray.cols*1;
    //     // todo only farne eye region
    //     GaussianBlur(gray, gray, cv::Size(0, 0), sigma);
    // }
/*
    diffclock("- facedetect", start);

    // start = clock();
    // eyeRegions(face, &leftEyeRegion, &rightEyeRegion);
    // diffclock("- eyeRegions", start);
    // start = clock();
    // eyeCenters(faceROI, leftEyeRegion, rightEyeRegion, &leftPupil, &rightPupil);
    // diffclock("- eyeCenters", start);
    start = clock();
    getLeftRightEyeMat(gray, leftEyeRegion, rightEyeRegion,  &left, &right);
    diffclock("- getLeftRightEyeMat", start);

*/
    if ((frameNum % 30) == 0) {
        // initialize
        flg=1;
        flg1=1;
        resetDelay=0;
        firstLoopProcs = 1;
//        pause=1;
    } else {
        resetDelay++;
    }
    if ((frameNum % 2) == 0) {
        return;
    }
    if (flg == 1) {
        if (faceDetect(gray, &face) != 0) {
            if (debug_show_img_main == true && PHONE == 0) {
                imshow("main", out);
            }
            return;
        }

        int rowsO = (face).height/4.3;
        int colsO = (face).width/5.5;
        int rows2 = (face).height/4.3;
        int cols2 = (face).width/3.7;
        leftE = cv::Rect(colsO, rowsO, cols2, rows2);
        rightE = cv::Rect((face).width-colsO-cols2, rowsO, cols2, rows2);

        this->num = 0;
        flg=0;
    }


    faceROI = gray(face);
    left  = faceROI(leftE);
    right = faceROI(rightE);

    //cv::equalizeHist(faceROI, faceROI);
/*
    GaussianBlur(left, left, cv::Size(3,3), 0);
    GaussianBlur(left, left, cv::Size(3,3), 0);
    cv::equalizeHist(left, left);
    cv::equalizeHist(right, right);
*/
    GaussianBlur(left, left, cv::Size(3,3), 0);
    GaussianBlur(left, left, cv::Size(3,3), 0);
    cv::equalizeHist(left, left);
    cv::equalizeHist(right, right);

    toSave = faceROI.clone();
/*
    eyeCenters(faceROI, leftE, rightE, leftPupil, rightPupil);
    leftPupil.x += leftE.x; leftPupil.y += leftE.y;
    rightPupil.x += rightE.x; rightPupil.y += rightE.y;
    leftPupil.x += face.x; leftPupil.y += face.y;
    rightPupil.x += face.x; rightPupil.y += face.y;

    circle(out, rightPupil, 3, cv::Scalar(0,255,0), -1, 8);
    circle(out, leftPupil, 3, cv::Scalar(0,255,0), -1, 8);
*/
    int noiseReduct=0;
    int noise2Fris=0;
    if (noiseReduct == 1) {
        GaussianBlur(faceROI, faceROI, cv::Size(3,3), 0);
    } else if (noiseReduct == 2) {
        if (this->num == 0) {
            if (noise2Fris==1) {
                this->oprev = faceROI.clone();
                this->opprev = faceROI.clone();
            } else {
                this->loprev = left.clone();
                this->lopprev = left.clone();
                this->roprev = right.clone();
                this->ropprev = right.clone();
            }
            this->num++;
        } else {
            cv::Mat result, lresult, rresult;
            std::vector<cv::Mat> original(3);
            if (noise2Fris==1) {
                original[0] = this->opprev;
                original[1] = this->oprev;
                original[2] = faceROI;
                cv::fastNlMeansDenoisingMulti(original, result, 3/2, 3, 10);
            } else {
                std::vector<cv::Mat> original(3);
                original[0] = this->lopprev;
                original[1] = this->loprev;
                original[2] = left;
                cv::fastNlMeansDenoisingMulti(original, lresult, 3/2, 3, 10);
                original[0] = this->ropprev;
                original[1] = this->roprev;
                original[2] = right;
                cv::fastNlMeansDenoisingMulti(original, rresult, 3/2, 3, 10);
            }

            if (noise2Fris==1) {
                this->opprev = this->oprev;
                this->oprev = faceROI.clone();
                faceROI = result;
                faceROI.copyTo(gray(face));
            } else {
                this->lopprev = this->loprev;
                this->loprev = left.clone();
                this->ropprev = this->roprev;
                this->roprev = right.clone();
                left = lresult;
                right = rresult;
                left.copyTo(gray(cv::Rect(face.x+leftE.x,face.y+leftE.y,leftE.width,leftE.height)));
                right.copyTo(gray(cv::Rect(face.x+rightE.x,face.y+rightE.y,rightE.width,rightE.height)));
                
            }
        }
    }
    imshowWrapper("left", left, debug_show_img_templ_eyes_tmpl);

    imshowWrapper("right", right, debug_show_img_templ_eyes_tmpl);


    imshowWrapper("face", faceROI, debug_show_img_face);
/*
    //cv::equalizeHist(left, left);
    //cv::equalizeHist(right, right);
    cv::threshold(left, left, 27, 255, CV_THRESH_BINARY);
    cv::threshold(right, right, 27, 255, CV_THRESH_BINARY);
    //cv::threshold(left, left, 50, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);
    //cv::threshold(right, right, 50, 255, CV_THRESH_BINARY+CV_THRESH_OTSU);
    //cv::adaptiveThreshold(left, left, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 15, 0);
    //cv::adaptiveThreshold(right, right, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 15, 0);
    imshowWrapper("leftSR", left, debug_show_img_templ_eyes_cor);
    imshowWrapper("rightSR", right, debug_show_img_templ_eyes_cor);

    cv::Mat lEdge, rEdge;
    cv::Canny(left, lEdge, 75, 150, 3);
    cv::Canny(right, rEdge, 75, 150, 3);
    imshowWrapper("leftR", lEdge, debug_show_img_templ_eyes_cor);
    imshowWrapper("rightR", rEdge, debug_show_img_templ_eyes_cor);
*/
/*
    std::vector<cv::Vec3f> hcircles;
    cv::HoughCircles(left, hcircles, CV_HOUGH_GRADIENT, 1, 100, 150, 9, 1, 30);
printf("hdetected num %lu\n",hcircles.size());
    for(size_t i = 0; i < hcircles.size(); i++) {
        cv::Vec3i c = hcircles[i];
        cv::circle(out, cv::Point(c[0]+face.x+leftE.x, c[1]+face.y+leftE.y), c[2], cv::Scalar(0,0,255), 3, CV_AA);
        cv::circle(out, cv::Point(c[0]+face.x+leftE.x, c[1]+face.y+leftE.y), 2, cv::Scalar(0,255,0), 3, CV_AA);
    }
*/
/*
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    /// Find contours
    cv::findContours(lEdge, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
        std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
        std::vector<cv::Rect> boundRect( contours.size() );
        std::vector<cv::Point2f>center( contours.size() );
        std::vector<float>radius( contours.size() );

    for(unsigned int i = 0; i < contours.size(); i++ ) {
        cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true );
       boundRect[i] = cv::boundingRect( cv::Mat(contours_poly[i]) );
       cv::minEnclosingCircle( (cv::Mat)contours_poly[i], center[i], radius[i] );
     }
//printf("detected num %lu\n",contours.size());
    /// Draw contours
    //cv::Mat drawing = cv::Mat::zeros( canny_output.size(), CV_8UC3 );
cv::RNG rng(12345);
    for(unsigned int i = 0; i< contours.size(); i++ ) {
        if (radius[i] < 6) {
            continue;
        }
        cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        cv::drawContours(out, contours, i, color, 2, 8, hierarchy, 0, cv::Point());
        cv::rectangle(out, boundRect[i].tl(), boundRect[i].br(), color, 1, 8, 0 );
        cv::circle(out, center[i], (int)radius[i], color, 1, 8, 0 );
 //       printf("radius %d\n",(int)radius[i]);
    }
*/

//imshowWrapper("main", out, debug_show_img_main);
//    return;
    //printf("face %d %d\n", face.width, face.height);
    //imshow("face", faceROI);
    //return;

    //cv::equalizeHist(faceROI, faceROI);
//    GaussianBlur(left, left, cv::Size(3,3), 0);
//    GaussianBlur(right, right, cv::Size(3,3), 0);


    if (firstLoopProcs == 0) {
        start = clock();
        cv::calcOpticalFlowFarneback(pleft, left, flowLeft, 0.5, 3, 15, 3, 5, 1.2, 0);
        cv::calcOpticalFlowFarneback(pright, right, flowRight, 0.5, 3, 15, 3, 5, 1.2, 0);
        diffclock("- farneback", start);

        start = clock();
        drawOptFlowMap(face, leftE, flowLeft, out, 5, cv::Scalar(0, 255, 0), 0);
        drawOptFlowMap(face, rightE, flowRight, out, 5, cv::Scalar(0, 255, 0), 1);
        diffclock("- drawOptFlowMap", start);

        if (debug_show_img_optfl_eyes == true && PHONE == 0) {
            imshow("left", left);
            imshow("right", right);
        }
    } else {
        firstLoopProcs = 0;
    }


/*
    if (resetDelay > 3) {
        if (flg1 == 1) {
            start = clock();

            printf("BUJU init %d\n", frameNum);
            cv::Mat lmask = cv::Mat::zeros(gray.size(), CV_8U);
            lmask(cv::Rect(face.x+leftE.x, face.y+leftE.y, leftE.width, leftE.height)) = 1;
            cv::goodFeaturesToTrack(gray, lpoints[1], MAX_COUNT, 0.05, 1, lmask, 3, false, 0.04);
            printf("BUJU lInit res %d %lu\n", frameNum, lpoints[1].size());

            cv::Mat rmask = cv::Mat::zeros(gray.size(), CV_8U);
            rmask(cv::Rect(face.x+rightE.x, face.y+rightE.y, rightE.width, rightE.height)) = 1;
            cv::goodFeaturesToTrack(gray, rpoints[1], MAX_COUNT, 0.05, 1, rmask, 3, false, 0.04);
            printf("BUJU rInit res %d %lu\n", frameNum, rpoints[1].size());
            // cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit); // sig abrt
            flg1 = 0;
         } else {
            if(!lpoints[0].empty()){
                cv::Point2f point;
                std::vector<uchar> status;
                std::vector<float> err;
                cv::TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
                cv::Size subPixWinSize(10,10), winSize(31,31);
                
                cv::calcOpticalFlowPyrLK(pgray, gray, lpoints[0], lpoints[1], status, err  , winSize, 3, termcrit, 0, 0.001);
                
                size_t i, k;
                for(i = k = 0; i < lpoints[1].size(); i++) {
                    if(addRemovePtx) {
                        if(norm(point - lpoints[1][i]) <= 5 ) {
                            addRemovePtx = false;
                            continue;
                        }
                    }
                    if(!status[i]) {
                        continue;
                    }
                    lpoints[1][k++] = lpoints[1][i];
                    cv::circle(out, lpoints[1][i], 1, cv::Scalar(0,255,0), -1, 8);
                    cv::circle(gray, lpoints[1][i], 1, cv::Scalar(0,255,0), -1, 8);
                }
                lpoints[1].resize(k);
            }
            if(!rpoints[0].empty()){
                cv::Point2f point;
                std::vector<uchar> status;
                std::vector<float> err;
                cv::TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
                cv::Size subPixWinSize(10,10), winSize(31,31);
                
                cv::calcOpticalFlowPyrLK(pgray, gray, rpoints[0], rpoints[1], status, err  , winSize, 3, termcrit, 0, 0.001);
                
                size_t i, k;
                for(i = k = 0; i < rpoints[1].size(); i++) {
                    if(addRemovePtx) {
                        if(norm(point - rpoints[1][i]) <= 5 ) {
                            addRemovePtx = false;
                            continue;
                        }
                    }
                    if(!status[i]) {
                        continue;
                    }
                    rpoints[1][k++] = rpoints[1][i];
                    cv::circle(out, rpoints[1][i], 1, cv::Scalar(0,255,0), -1, 8);
                    cv::circle(gray, rpoints[1][i], 1, cv::Scalar(0,255,0), -1, 8);
                }
                rpoints[1].resize(k);
            }
        }
        std::swap(lpoints[1], lpoints[0]);
        std::swap(rpoints[1], rpoints[0]);
    }
*/
//    imshowWrapper("gray", gray, debug_show_img_main);

    pleft = left.clone(); pright = right.clone();
    pgray = gray.clone();
    start = clock();
    showResult(out, face, faceROI, leftEyeRegion, rightEyeRegion, leftPupil, rightPupil);
    diffclock("- showResult", start);

}

OptFlow::OptFlow () {
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-startx).count();
    
    termcrit= cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
    subPixWinSize = cv::Size(10,10);
    winSize = cv::Size(31,31);
};

int OptFlow::setup(const char* cascadeFileName) {
    try {
        if(!face_cascade.load(cascadeFileName)) {
            throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
        }
    } catch (const char* msg) {
        doLog(true, msg);
        throw;
    }
    return 0;
};

#ifdef IS_PHONE
int OptFlow::setJni(JNIEnv* jenv) {
};
#endif

int OptFlow::run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    //cvtColor(rgb, grayx, COLOR_BGR2GRAY);
    //process(rgb, grayx, rgb);
    process(gray, out, timestamp, frameNum);

    //cv::swap(prevLeft, left);
    //cv::swap(prevRight, right);
    return 0;
};

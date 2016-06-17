
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
#include <farneback.hpp>

void fbDrawOptFlowMap (cv::Rect face, cv::Rect eyeE, const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye) {
    cv::circle(cflowmap, cv::Point2f((float)15, (float)15), 10, cv::Scalar(0,255,0), -1, 8);
    int xo, yo;
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



int Farneback::faceDetect(cv::Mat gray, cv::Rect *face) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
    if (faces.size() != 1) {
        return -1;
    }
    *face = faces[0];
    return 0;
}
void Farneback::eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point &leftPupil, cv::Point &rightPupil) {
    leftPupil  = findEyeCenter(faceROI, leftEyeRegion);
    rightPupil = findEyeCenter(faceROI, rightEyeRegion);
}
bool Farneback::preprocess(cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum) {
    GaussianBlur(left, left, cv::Size(3,3), 0);
    GaussianBlur(left, left, cv::Size(3,3), 0);
    cv::equalizeHist(left, left);
    cv::equalizeHist(right, right);
}
bool Farneback::reinit(cv::Mat gray, cv::Mat& faceROI, cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum) {
    int fdRes = this->faceDetect(gray, &this->faceRef);

    if (fdRes != 0) {
        return false;
    }

    faceROI = gray(this->faceRef);
    imshowWrapper("face", faceROI, debug_show_img_face);

    // pupil search region definition TODO make it bigger
    int rowsO = (this->faceRef).height/4.3;
    int colsO = (this->faceRef).width/5.5;
    int rows2 = (this->faceRef).height/4.3;
    int cols2 = (this->faceRef).width/3.7;
    // int rowsO = (face).height/4.3;
    // int colsO = (face).width/5.5;
    // int rows2 = (face).height/4.3;
    // int cols2 = (face).width/3.7;
    this->leftE = cv::Rect(colsO, rowsO, cols2, rows2);
    this->rightE = cv::Rect((this->faceRef).width-colsO-cols2, rowsO, cols2, rows2);

    // preprocess only eye region(blur, eqHist)
    left  = faceROI(this->leftE);
    right = faceROI(this->rightE);
    this->preprocess(left, right, timestamp, frameNum);

    // locate and save pupil location
    this->eyeCenters(faceROI, this->leftE, this->rightE, this->lEye, this->rEye);
    //this->lEye.x = leftPupil.x; this->lEye.y = leftPupil.y;
    //this->rEye.x = rightPupil.x; this->rEye.y = rightPupil.y;
    this->lLastTime = timestamp;
    this->rLastTime = timestamp;

    // farneback region definition derived from pupil location TODO update based on search region
    this->leftE = cv::Rect(colsO, rowsO, cols2, rows2);
    this->rightE = cv::Rect((this->faceRef).width-colsO-cols2, rowsO, cols2, rows2);

    doLog(debug_fb_log1, "debug_fb_log1: F %u T %.3lf lEye %d %d rEye %d %d lLastTime %lf rLastTime %lf\n",
        frameNum, timestamp, this->lEye.x, this->lEye.y, this->rEye.x, this->rEye.y, this->lLastTime, this->rLastTime);

    return true;
}
void Farneback::updateSearch(cv::Mat gray, cv::Rect& lTemplSearchR, cv::Rect& rTemplSearchR, cv::Mat& lTemplSearch, cv::Mat& rTemplSearch) {
    //TODO check eyeCenters every n ms (if farneback didn't recognire much movement)
/*    int lColsHalf = leftE.width/2;
    int lRowsHalf = leftE.height/2;
    int rColsHalf = rightE.width/2;
    int rRowsHalf = rightE.height/2;
    int lSearchSX = this->lEye.x-lColsHalf;
    int rSearchSX = this->rEye.x-rColsHalf;
    int lSearchSY = this->lEye.y-lRowsHalf;
    int rSearchSY = this->rEye.y-rRowsHalf;
    lSearchR = cv::Rect(lSearchSX, lSearchSY, leftE.width*1.8, leftE.height*2);
    rSearchR = cv::Rect(rSearchSX, rSearchSY, rightE.width*1.8, rightE.height*2);
    doLog(debug_tmpl_log, "debug_tmpl_log: AAA lTemplSearchR %d %d %d %d\n", lTemplSearchR.x, lTemplSearchR.y, lTemplSearchR.width, lTemplSearchR.height);
    doLog(debug_tmpl_log, "debug_tmpl_log: AAA rTemplSearchR %d %d %d %d\n", rTemplSearchR.x, rTemplSearchR.y, rTemplSearchR.width, rTemplSearchR.height);
    lTemplSearch = gray(lTemplSearchR);
    rTemplSearch = gray(rTemplSearchR);
    lTemplSearchR = cv::Rect(leftE.x, leftE.y, this->leftTemplate.cols*1.8, this->leftTemplate.rows*2);
    rTemplSearchR = cv::Rect(rTemplSearchSX, rTemplSearchSY, this->rightTemplate.cols*1.8, this->rightTemplate.rows*2);
*/
}
void Farneback::rePupil() {
    // this->eyeCenters(faceROI, leftE, rightE, leftPupil, rightPupil);

    // leftPupil.x += leftE.x; leftPupil.y += leftE.y;
    // rightPupil.x += rightE.x; rightPupil.y += rightE.y;
    // leftPupil.x += face.x; leftPupil.y += face.y;
    // rightPupil.x += face.x; rightPupil.y += face.y;

    // circle(out, rightPupil, 3, cv::Scalar(0,255,0), -1, 8);
    // circle(out, leftPupil, 3, cv::Scalar(0,255,0), -1, 8);
}
void Farneback::method(cv::Mat gray, cv::Mat& faceROI, cv::Mat& left, cv::Mat& right, cv::Mat& flowLeft, cv::Mat& flowRight) {
    cv::Rect leftB, rightB;
    cv::Mat pleft, pright, pfaceROI;

    pfaceROI = this->pgray(this->faceRef);
    faceROI = gray(this->faceRef);
    pleft = pfaceROI(this->leftE);
    pright = pfaceROI(this->rightE);
    left = faceROI(this->leftE);
    right = faceROI(this->rightE);

    cv::calcOpticalFlowFarneback(pleft, left, flowLeft, 0.5, 3, 15, 3, 5, 1.2, 0);
    cv::calcOpticalFlowFarneback(pright, right, flowRight, 0.5, 3, 15, 3, 5, 1.2, 0);

    // // bounding boxes
    // int leftBw = leftE.width*0.75, leftBh = leftE.height*0.4;
    // int rightBw = rightE.width*0.75, rightBh = rightE.height*0.4;
    // leftB = cv::Rect(leftPupil.x-(leftBw/2), leftPupil.y-(leftBh/2), leftBw, leftBh);
    // rightB = cv::Rect(rightPupil.x-(rightBw/2), rightPupil.y-(rightBh/2), rightBw, rightBh);
    // // draw eyes bounding boxes
    // cv::RNG rng(12345);
    // cv::Scalar coolor = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
    // cv::rectangle(out, cv::Rect(face.x+leftE.x+leftB.x,face.y+leftE.y+leftB.y,leftB.width,leftB.height), coolor, 1, 8, 0);
    // cv::rectangle(out, cv::Rect(face.x+rightE.x+rightB.x,face.y+rightE.y+rightB.y,rightB.width,rightB.height), coolor, 1, 8, 0);
    // cv::rectangle(left, cv::Rect(leftB.x,leftB.y,leftB.width,leftB.height), coolor, 1, 8, 0);
    // cv::rectangle(right, cv::Rect(rightB.x,rightB.y,rightB.width,rightB.height), coolor, 1, 8, 0);
}
void Farneback::process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    cv::Mat faceROI, left, right, flowLeft, flowRight;

    if (flagReinit == true) {
        if (this->reinit(gray, faceROI, left, right, timestamp, frameNum) != true) {
            doLog(debug_fb_log1, "debug_fb_log1: reinit failed frameNum %u timestamp %lf\n", frameNum, timestamp);
            return;
        } else {
            this->flagReinit = false;
        }
    } else {
        this->method(gray, faceROI, left, right, flowLeft, flowRight);
        fbDrawOptFlowMap(this->faceRef, this->leftE, flowLeft, out, 5, cv::Scalar(0, 255, 0), 0);
        fbDrawOptFlowMap(this->faceRef, this->rightE, flowRight, out, 5, cv::Scalar(0, 255, 0), 1);
    }

    if ((frameNum % 30) == 0) {
        this->flagReinit = true;
    }

    this->pgray = gray.clone();
    imshowWrapper("left", left, debug_show_img_templ_eyes_tmpl);
    imshowWrapper("right", right, debug_show_img_templ_eyes_tmpl);
    imshowWrapper("face", faceROI, debug_show_img_face);
    imshow("main", out);
    return;
    //cv::Point leftPupil, rightPupil;
}

Farneback::Farneback () {
};

int Farneback::setup(const char* cascadeFileName) {
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
int Farneback::setJni(JNIEnv* jenv) {
};
#endif

int Farneback::run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    //cvtColor(rgb, grayx, COLOR_BGR2GRAY);
    //process(rgb, grayx, rgb);
    this->process(gray, out, timestamp, frameNum);

    //cv::swap(prevLeft, left);
    //cv::swap(prevRight, right);
    return 0;
};


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

#include "eyeLike/src/findEyeCenter.h"


#include <common.hpp>
#include <farneback.hpp>

void Farneback::drawOptFlowMap (cv::Rect eyeE, const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye) {
    cv::circle(cflowmap, cv::Point2f((float)15, (float)15), 10, cv::Scalar(0,255,0), -1, 8);
    int xo, yo;
    xo = eyeE.x;
    yo = eyeE.y;
    for(int y = 0; y < flow.rows; y += step) {
        for(int x = 0; x < flow.cols; x += step) {
            const cv::Point2f& fxy = flow.at< cv::Point2f>(y, x);
            int px = x+xo, py = y+yo;
            cv::line(cflowmap, cv::Point(px,py), cv::Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), color);
            //circle(cflowmap, Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), 1, color, -1);
            cv::circle(cflowmap, cv::Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), 1, color, -1, 8);
            //circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
        }
    }
    // for(int y = 0; y < flow.rows; y += step) {
    //     for(int x = 0; x < flow.cols; x += step) {
    //         const cv::Point2f& fxy = flow.at< cv::Point2f>(y, x);
    //         printf("%.0f ", fxy.y);
    //     }
    //     printf("\n");
    // }
    // printf("\n\n\n");
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
bool Farneback::reinit(cv::Mat gray, cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum) {
    cv::Rect face;
    int fdRes = this->faceDetect(gray, &face);

    if (fdRes != 0) {
        return false;
    }

    // farneback region definition derived from face size and location
    int rowsO = face.height/4.3;
    int colsO = face.width/5.5;
    int rows2 = face.height/4.3;
    int cols2 = face.width/3.7;

    this->leftRg = cv::Rect(colsO, rowsO, cols2, rows2);
    this->rightRg = cv::Rect(face.width-colsO-cols2, rowsO, cols2, rows2);
    this->leftRg.x += face.x; this->leftRg.y += face.y;
    this->rightRg.x += face.x; this->rightRg.y += face.y;

    // pupil search region definition
    //this->leftSr = cv::Rect(this->leftRg.x-(this->leftRg.width*0.15), this->leftRg.y-(this->leftRg.height*0.3), this->leftRg.width*1.25, this->leftRg.height*1.5);
    //this->rightSr = cv::Rect(this->rightRg.x-(this->rightRg.width*0.15), this->rightRg.y-(this->rightRg.height*0.3), this->rightRg.width*1.25, this->rightRg.height*1.5);
    this->leftSr  = this->leftRg;
    this->rightSr = this->rightRg;

    // preprocess only eye region(blur, eqHist)
    this->preprocess(left, right, timestamp, frameNum);
    left  = gray(this->leftRg).clone();
    right = gray(this->rightRg).clone();

    if (debug_show_img_templ_eyes_tmpl == true) {
        cv::Mat leftSr, rightSr;
        leftSr = gray(this->leftSr); rightSr = gray(this->rightSr);
        imshowWrapper("leftSR", leftSr, debug_show_img_templ_eyes_tmpl);
        imshowWrapper("rightSR", rightSr, debug_show_img_templ_eyes_tmpl);
    }

    // locate and save pupil location
    this->eyeCenters(gray, this->leftSr, this->rightSr, this->lEye, this->rEye);
    this->lLastTime = timestamp;
    this->rLastTime = timestamp;

    cv::Mat faceROI = gray(face);
    imshowWrapper("face", faceROI, debug_show_img_face);

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
void Farneback::rePupil(cv::Mat gray, double timestamp, unsigned int frameNum) {
    cv::Point newLEyeLoc, newREyeLoc;
    cv::Mat leftSr, rightSr;
    leftSr = gray(this->leftSr); rightSr = gray(this->rightSr);
    //this->preprocess(leftSr, rightSr, timestamp, frameNum);
    imshowWrapper("leftSR", leftSr, debug_show_img_templ_eyes_tmpl);
    imshowWrapper("rightSR", rightSr, debug_show_img_templ_eyes_tmpl);

    this->eyeCenters(gray, this->leftSr, this->rightSr, newLEyeLoc, newREyeLoc);
/*
    if (newLEyeLoc.x < (this->leftE.width*0.3) || newLEyeLoc.x > (this->leftE.width*0.7)
        || newLEyeLoc.y < (this->leftE.height*0.3) || newLEyeLoc.y > (this->leftE.height*0.7)) {
        // TODO window edges
        this->leftE.x = newLEyeLoc.x - (this->leftE.width/2);
        this->leftE.y = newLEyeLoc.y + (this->leftE.height/2);
        pause = 1;
    }
*/
    // if (newREyeLoc.x < (this->rightE.width*0.3) || newREyeLoc.x > (this->rightE.width*0.7)
    //     || newREyeLoc.y < (this->rightE.height*0.3) || newREyeLoc.y > (this->rightE.height*0.7)) {
    //     // TODO window edges
    //     this->rightE.x = newLEyeLoc.x;
    //     this->rightE.y = newLEyeLoc.y;
    //     this->rightE.x -= (this->rightE.width/2);
    //     this->rightE.y -= (this->rightE.height/2);
    // }
    this->lEye = newLEyeLoc;
    this->rEye = newREyeLoc;

    // update eye locs
//    this->rePupil(faceROI);
    // cv::Point leftUpdateLoc, rightUpdateLoc;
    // this->dominantDirection(flowLeft, leftUpdateLoc);
    // this->dominantDirection(flowRight, rightUpdateLoc);
    // this->lEye.x += leftUpdateLoc.x;
    // this->lEye.y += leftUpdateLoc.y;
    // this->rEye.x += rightUpdateLoc.x;
    // this->rEye.y += rightUpdateLoc.y;
    // printf("%d,%d %d,%d \n", leftUpdateLoc.x, leftUpdateLoc.y, rightUpdateLoc.x, rightUpdateLoc.y);

}
void Farneback::dominantDirection(cv::Mat flow, cv::Point& updateLoc) {
    double totalX=0, totalY=0;
    for(int y = 0; y < flow.rows; y += 1) {
        for(int x = 0; x < flow.cols; x += 1) {
            const cv::Point2f& flowVector = flow.at<cv::Point2f>(y, x);
            totalX += flowVector.x;
            totalY += flowVector.y;
        }
    }
    totalX /= flow.cols;
    totalY /= flow.cols;
    totalX /= flow.rows;
    totalY /= flow.rows;
    updateLoc = cv::Point(totalX, totalY);
}
void Farneback::method(cv::Mat gray, cv::Mat& left, cv::Mat& right, cv::Mat& flowLeft, cv::Mat& flowRight, cv::Rect& leftB, cv::Rect& rightB, double timestamp, unsigned int frameNum) {
    left = gray(this->leftRg);
    right = gray(this->rightRg);

    // preprocess only eye region(blur, eqHist)
    this->preprocess(left, right, timestamp, frameNum);
    left = left.clone();
    right = right.clone();

    cv::calcOpticalFlowFarneback(this->pleft, left, flowLeft, 0.5, 3, 15, 3, 5, 1.2, 0);
    cv::calcOpticalFlowFarneback(this->pright, right, flowRight, 0.5, 3, 15, 3, 5, 1.2, 0);

    // bounding boxes
    int leftBw = this->leftRg.width*0.75, leftBh = this->leftRg.height*0.4;
    int rightBw = this->rightRg.width*0.75, rightBh = this->rightRg.height*0.4;
    leftB = cv::Rect(this->lEye.x-(leftBw/2), this->lEye.y-(leftBh/2), leftBw, leftBh);
    rightB = cv::Rect(this->rEye.x-(rightBw/2), this->rEye.y-(rightBh/2), rightBw, rightBh);
}
void Farneback::process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    cv::Mat left, right, flowLeft, flowRight;
    cv::Rect leftB, rightB;

    if (flagReinit == true) {
        if (this->reinit(gray, left, right, timestamp, frameNum) != true) {
            doLog(debug_fb_log1, "debug_fb_log1: reinit failed frameNum %u timestamp %lf\n", frameNum, timestamp);
            return;
        } else {
            this->flagReinit = false;
        }
    } else {
        if ((frameNum % 2) == 0) {
            return;
        }
        this->method(gray, left, right, flowLeft, flowRight, leftB, rightB, frameNum, timestamp);
        this->drawOptFlowMap(this->leftRg, flowLeft, out, 5, cv::Scalar(0, 255, 0), 0);
        this->drawOptFlowMap(this->rightRg, flowRight, out, 5, cv::Scalar(0, 255, 0), 1);
        this->rePupil(gray, frameNum, timestamp);
    }

    if ((frameNum % 30) == 0) {
//        this->flagReinit = true;
    }

    // draw
    cv::Point lTmp = cv::Point(this->leftSr.x+this->lEye.x, this->leftSr.y+this->lEye.y);
    cv::Point rTmp = cv::Point(this->rightSr.x+this->rEye.x, this->rightSr.y+this->rEye.y);
    circle(out, lTmp, 3, cv::Scalar(0,255,0), -1, 8);
    circle(out, rTmp, 3, cv::Scalar(0,255,0), -1, 8);

    // draw eyes bounding boxes
    cv::RNG rng(12345);
    cv::Scalar coolor = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
    cv::rectangle(out, cv::Rect(this->leftRg.x+leftB.x, this->leftRg.y+leftB.y, leftB.width, leftB.height), coolor, 1, 8, 0);
    cv::rectangle(out, cv::Rect(this->rightRg.x+rightB.x, this->rightRg.y+rightB.y, rightB.width, rightB.height), coolor, 1, 8, 0);

    imshowWrapper("left", left, debug_show_img_templ_eyes_tmpl);
    imshowWrapper("right", right, debug_show_img_templ_eyes_tmpl);
    imshow("main", out);

    this->pleft = left;
    this->pright = right;
    return;
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

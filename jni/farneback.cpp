
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
#include <blinkmeasuref.hpp>

void Farneback::drawOptFlowMap (cv::Rect eyeE, const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, __attribute__((unused)) int eye) {
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
bool Farneback::preprocess(cv::Mat& left, cv::Mat& right, __attribute__((unused)) double timestamp, __attribute__((unused)) unsigned int frameNum) {
    GaussianBlur(left, left, cv::Size(3,3), 0);
    GaussianBlur(left, left, cv::Size(3,3), 0);
    cv::equalizeHist(left, left);
    cv::equalizeHist(right, right);
/*
*/
    return true;
}
bool Farneback::reinit(cv::Mat gray, cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum) {
    cv::Rect face;
    std::chrono::time_point<std::chrono::steady_clock> t1;

    t1 = std::chrono::steady_clock::now();
    int fdRes = this->faceDetect(gray, &face);
    difftime("debug_fb_perf2: reinit:faceDetect", t1, debug_fb_perf2);

    if (fdRes != 0) {
        return false;
    }

    // farneback region definition derived from face size and location
    int rowsO = face.height/4.3;
    int colsO = face.width/5.5;
    int rows2;
    if (this->onlyLower == true) {
        rows2 = face.height/4.0;
    } else {
        rows2 = face.height/4.3;
    }
    int cols2 = face.width/3.7;

    this->leftRg = cv::Rect(colsO, rowsO, cols2, rows2);
    this->rightRg = cv::Rect(face.width-colsO-cols2, rowsO, cols2, rows2);
    this->leftRg.x += face.x; this->leftRg.y += face.y;
    this->rightRg.x += face.x; this->rightRg.y += face.y;

    // preprocess only eye region(blur, eqHist)
    t1 = std::chrono::steady_clock::now();
    this->preprocess(left, right, timestamp, frameNum);
    difftime("debug_fb_perf2: reinit:preprocess", t1, debug_fb_perf2);
    left  = gray(this->leftRg).clone();
    right = gray(this->rightRg).clone();

    // locate and save pupil location
    t1 = std::chrono::steady_clock::now();
    this->eyeCenters(gray, this->leftRg, this->rightRg, this->lEye, this->rEye);
    difftime("debug_fb_perf2: reinit:eyeCenters", t1, debug_fb_perf2);
    this->lastRepupilTime = timestamp;
    this->lastRepupilDiffLeft = cv::Point2d(0,0);
    this->lastRepupilDiffRight = cv::Point2d(0,0);
    this->initEyesDistance = (this->rightRg.x+this->rEye.x)-(this->leftRg.x+this->lEye.x);
    this->lLastTime = timestamp;
    this->rLastTime = timestamp;
    this->canCallMeasureBlinks = true;
    blinkMeasuref.clear();

    cv::Mat faceROI = gray(face);
    imshowWrapper("face", faceROI, debug_show_img_face);

    doLog(debug_fb_log_reinit, "debug_fb_log_reinit: F %u T %.3lf lEye %d %d rEye %d %d lLastTime %lf rLastTime %lf\n",
        frameNum, timestamp, this->lEye.x, this->lEye.y, this->rEye.x, this->rEye.y, this->lLastTime, this->rLastTime);
    //printf("reinit F %u T %.3lf\n", frameNum, timestamp);

    return true;
}

std::array<bool, 4> Farneback::rePupil(cv::Mat gray, double timestamp, unsigned int frameNum) {
    std::chrono::time_point<std::chrono::steady_clock> t1;
    bool firePreprocess = false, canUpdateL = false, canUpdateR = false;
    cv::Point newLEyeLoc, newREyeLoc;
    unsigned int curXEyesDistance, curYEyesDistance;
    const int maxDiff = 15;
    int proceedDelay = 250;
    bool canProceedL = true, canProceedR = true;
    if ((this->lLastTime+proceedDelay) > timestamp && (this->rLastTime+proceedDelay) > timestamp && (this->lastRepupilTime+proceedDelay) > timestamp
        && abs(this->lastRepupilDiffLeft.x) < 2 && abs(this->lastRepupilDiffLeft.y) < 2
        && abs(this->lastRepupilDiffRight.x) < 2 && abs(this->lastRepupilDiffRight.y) < 2
        ) {
        //printf("%5.2lf\t%5.2lf\t%5.2lf\t%5.2lf\n", this->lastRepupilDiffLeft.x, this->lastRepupilDiffLeft.y, this->lastRepupilDiffRight.x, this->lastRepupilDiffRight.y);
        std::array<bool, 4> ret;
        ret[0] = true;
        ret[1] = true;
        ret[2] = true;
        ret[3] = true;
        return ret;
    }
    //printf("time since last repupil %.2lf\n", timestamp-this->lastRepupilTime);
    this->lastRepupilTime = timestamp;
    t1 = std::chrono::steady_clock::now();
    this->eyeCenters(gray, this->leftRg, this->rightRg, newLEyeLoc, newREyeLoc);
    difftime("debug_fb_perf2: rePupil:eyeCenters", t1, debug_fb_perf2);
    doLog(debug_fb_log_repupil1, "debug_fb_log_repupil1: F %u T %lf L diff x %d y %d\n", frameNum, timestamp, newLEyeLoc.x-this->lEye.x, newLEyeLoc.y-this->lEye.y);
    doLog(debug_fb_log_repupil1, "debug_fb_log_repupil1: F %u T %lf R diff x %d y %d\n", frameNum, timestamp, newREyeLoc.x-this->rEye.x, newREyeLoc.y-this->rEye.y);
    this->lastRepupilDiffLeft = cv::Point2d(0,0);
    this->lastRepupilDiffRight = cv::Point2d(0,0);
    if ((abs(newLEyeLoc.x-this->lEye.x)+abs(newLEyeLoc.y-this->lEye.y)) < maxDiff) {
            this->lLastTime = timestamp;
            canUpdateL = true;
    }
    if ((abs(newREyeLoc.x-this->rEye.x)+abs(newREyeLoc.y-this->rEye.y)) < maxDiff) {
            this->rLastTime = timestamp;
            canUpdateR = true;
    }

    // current eye's positions are geometrically unlikely to be correct, so request reinit
    if (canUpdateL == true && canUpdateR == true) {
        curXEyesDistance = (this->rightRg.x+newREyeLoc.x)-(this->leftRg.x+newLEyeLoc.x);
        curYEyesDistance = abs((this->rightRg.y+newREyeLoc.y)-(this->leftRg.y+newLEyeLoc.y));
        if (curXEyesDistance < (this->initEyesDistance*0.75)
            || curXEyesDistance > (this->initEyesDistance*1.30)
            || curYEyesDistance > this->initEyesDistance*0.30) {
            doLog(debug_fb_log1, "debug_fb_log1: F %u T %lf initEyesDistance %u curXEyesDistance %u curYEyesDistance %u\n",
                frameNum, timestamp, this->initEyesDistance, curXEyesDistance, curYEyesDistance);
            this->flagReinit = true;
        }
    } else {
        doLog(debug_fb_log_repupil, "debug_fb_log_repupil: F %u T %lf L %d R %d\n", frameNum, timestamp, canUpdateL?1:0, canUpdateR?1:0);
    }

    //this->canCallMeasureBlinks = (canUpdateL == true && canUpdateR == true);
    this->canCallMeasureBlinks = (canUpdateL == true && canUpdateR == true && this->flagReinit == false);

    //printf("L repupil F %u T %.3lf %d reinit %d canCallMeasureBlinks %d diff %3d %3d\n", frameNum, timestamp, canUpdateL?1:0, this->flagReinit?1:0, this->canCallMeasureBlinks?1:0, newLEyeLoc.x-this->lEye.x, newLEyeLoc.y-this->lEye.y);
    //printf("R repupil F %u T %.3lf %d reinit %d canCallMeasureBlinks %d diff %3d %3d\n", frameNum, timestamp, canUpdateR?1:0, this->flagReinit?1:0, this->canCallMeasureBlinks?1:0, newREyeLoc.x-this->rEye.x, newREyeLoc.y-this->rEye.y);
    // if we lost eyes for more than half a second, request reinit
    if ((this->lLastTime+400) < timestamp || (this->rLastTime+400) < timestamp) {
        // we lost eyes, request reinit
        this->flagReinit = true;
        doLog(debug_fb_log1, "debug_fb_log1: F %u T %lf reinit: eyes were displaced lLastTime %lf rLastTime %lf\n", frameNum, timestamp, this->lLastTime, this->rLastTime);
    } else {
        doLog(debug_fb_log1, "debug_fb_log1: F %u T %lf diff L %lf R %lf\n", frameNum, timestamp, timestamp-this->lLastTime, timestamp-this->rLastTime);
    }
    double pupilIdealY;
    double pupilLowerLimit;
    if (this->onlyLower == true) {
        pupilIdealY = 2.5;
        pupilLowerLimit = 0.6;
    } else {
        pupilIdealY = 2;
        pupilLowerLimit = 0.7;
    }
    // reposition pupil location and optionaly farneback grid
    if (canUpdateL == true
        && (newLEyeLoc.x < (this->leftRg.width*0.3) || newLEyeLoc.x > (this->leftRg.width*0.7)
            || newLEyeLoc.y < (this->leftRg.height*0.3) || newLEyeLoc.y > (this->leftRg.height*pupilLowerLimit))) {
        // reposition leftRg so that lEye will be in the middle
        unsigned int idealX = this->leftRg.width/2, idealY = this->leftRg.height/pupilIdealY;
        int moveX = newLEyeLoc.x-idealX, moveY = newLEyeLoc.y-idealY;
        if ((this->leftRg.x + moveX) < 0 || (this->leftRg.y + moveY) < 0
            || (this->leftRg.x + moveX + this->leftRg.width) > gray.cols
            || (this->leftRg.y + moveY + this->leftRg.height) > gray.rows) {
            this->flagReinit = true;
        } else {
            //doLog(debug_fb_log1, "debug_fb_log1: pL %u %u , %u %u\n", this->leftRg.x, this->leftRg.y, newLEyeLoc.x, newLEyeLoc.y);
            this->leftRg.x += moveX; this->leftRg.y += moveY;
            // update newLEyeLoc because we changed leftRg's location
            newLEyeLoc.x -= moveX; newLEyeLoc.y -= moveY;
            //doLog(debug_fb_log1, "debug_fb_log1: aL %u %u , %u %u\n", this->leftRg.x, this->leftRg.y, newLEyeLoc.x, newLEyeLoc.y);
            firePreprocess = true;
            canProceedL = false;
        }
    }
    if (canUpdateR == true
        && (newREyeLoc.x < (this->rightRg.width*0.3) || newREyeLoc.x > (this->rightRg.width*0.7)
            || newREyeLoc.y < (this->rightRg.height*0.3) || newREyeLoc.y > (this->rightRg.height*pupilLowerLimit))) {
        // reposition rightRg so that rEye will be in the middle
        unsigned int idealX = this->rightRg.width/2, idealY = this->rightRg.height/pupilIdealY;
        int moveX = newREyeLoc.x-idealX, moveY = newREyeLoc.y-idealY;
        if ((this->rightRg.x + moveX) < 0 || (this->rightRg.y + moveY) < 0
            || (this->rightRg.x + moveX + this->rightRg.width) > gray.cols
            || (this->rightRg.y + moveY + this->rightRg.height) > gray.rows) {
            this->flagReinit = true;
        } else {
            //doLog(debug_fb_log1, "debug_fb_log1: pR %u %u , %u %u\n", this->rightRg.x, this->rightRg.y, newLEyeLoc.x, newLEyeLoc.y);
            this->rightRg.x += moveX; this->rightRg.y += moveY;
            // update newREyeLoc because we changed rightRg's location
            newREyeLoc.x -= moveX; newREyeLoc.y -= moveY;
            //doLog(debug_fb_log1, "debug_fb_log1: aR %u %u , %u %u\n", this->rightRg.x, this->rightRg.y, newLEyeLoc.x, newLEyeLoc.y);
            firePreprocess = true;
            canProceedR = false;
        }
    }
    if (firePreprocess == true) {
        cv::Mat leftRg = gray(this->leftRg).clone();
        cv::Mat rightRg = gray(this->rightRg).clone();
        t1 = std::chrono::steady_clock::now();
        this->preprocess(leftRg, rightRg, timestamp, frameNum);
        difftime("debug_fb_perf2: rePupil:preprocess", t1, debug_fb_perf2);
        this->pleft = leftRg; this->pright = rightRg;
        //pause = 1;
    }

    if (canUpdateL == true) {
        this->lEye = newLEyeLoc;
    }
    if (canUpdateR == true) {
        this->rEye = newREyeLoc;
    }
    std::array<bool, 4> ret;
    ret[0] = canProceedL;
    ret[1] = canProceedR;
    ret[2] = canUpdateL;
    ret[3] = canUpdateR;
    return ret;

    // if (newREyeLoc.x < (this->rightE.width*0.3) || newREyeLoc.x > (this->rightE.width*0.7)
    //     || newREyeLoc.y < (this->rightE.height*0.3) || newREyeLoc.y > (this->rightE.height*0.7)) {
    //     // TODO window edges
    //     this->rightE.x = newLEyeLoc.x;
    //     this->rightE.y = newLEyeLoc.y;
    //     this->rightE.x -= (this->rightE.width/2);
    //     this->rightE.y -= (this->rightE.height/2);
    // }

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
void Farneback::dominantDirection(cv::Mat flow, cv::Rect bounding, cv::Point2d& totalP, cv::Point2d& boundingP, cv::Point2d& diffP) {
    cv::Point2d upper=cv::Point2d(0, 0), lower=cv::Point2d(0, 0);
    int upperPointsNum = 0, lowerPointsNum = 0;
    double totalX=0, totalY=0, btotalX=0, btotalY=0;
    for(int y = 0; y < flow.rows; y += 1) {
        for(int x = 0; x < flow.cols; x += 1) {
            const cv::Point2f& flowVector = flow.at<cv::Point2f>(y, x);
            if (x >= bounding.x && x < (bounding.x+bounding.width)
                && y >= bounding.y && y < (bounding.y+bounding.height)) {
                btotalX += flowVector.x;
                btotalY += flowVector.y;
            } else {
                totalX += flowVector.x;
                totalY += flowVector.y;
                if (y < bounding.y) {
                    upper.x += flowVector.x;
                    upper.y += flowVector.y;
                    upperPointsNum++;
                } else {
                    lower.x += flowVector.x;
                    lower.y += flowVector.y;
                    lowerPointsNum++;
                }
            }
        }
    }
    totalX /= (flow.cols*flow.rows-bounding.width*bounding.height);
    totalY /= (flow.cols*flow.rows-bounding.width*bounding.height);
    btotalX /= (bounding.width*bounding.height);
    btotalY /= (bounding.width*bounding.height);
    if (lowerPointsNum > 0) {
        lower.x /= lowerPointsNum;
        lower.y /= lowerPointsNum;
    }
    if (upperPointsNum > 0) {
        upper.x /= upperPointsNum;
        upper.y /= upperPointsNum;
    }
    totalP = cv::Point2d(totalX, totalY);
    boundingP = cv::Point2d(btotalX, btotalY);
    if (this->onlyLower == true) {
        diffP = cv::Point2d(lower.x-btotalX, lower.y-btotalY);
    } else {
        diffP = cv::Point2d(totalX-btotalX, totalY-btotalY);
    }
}
void Farneback::method(cv::Mat gray, bool canProceedL, bool canProceedR, bool canUpdateL, bool canUpdateR, cv::Mat& left, cv::Mat& right, cv::Mat& flowLeft, cv::Mat& flowRight, cv::Rect& leftB, cv::Rect& rightB, double timestamp, unsigned int frameNum) {
    std::chrono::time_point<std::chrono::steady_clock> t1;
    cv::Point2d lTotalP, lBoundingP, lDiffP, rTotalP, rBoundingP, rDiffP;
    left = gray(this->leftRg);
    right = gray(this->rightRg);


    // preprocess only eye region(blur, eqHist)
    t1 = std::chrono::steady_clock::now();
    this->preprocess(left, right, timestamp, frameNum);
    difftime("debug_fb_perf2: method:preprocess", t1, debug_fb_perf2);
    left = left.clone();
    right = right.clone();

    if (canProceedL == true) {
        t1 = std::chrono::steady_clock::now();
        cv::calcOpticalFlowFarneback(this->pleft, left, flowLeft, 0.5, 3, 15, 3, 5, 1.2, 0);
        difftime("debug_fb_perf2: method:calcOpticalFlowFarnebackL", t1, debug_fb_perf2);
        t1 = std::chrono::steady_clock::now();
        int leftBw = this->leftRg.width*0.75, leftBh = this->leftRg.height*0.4;
        leftB = cv::Rect(this->lEye.x-(leftBw/2), this->lEye.y-(leftBh/2), leftBw, leftBh);
        this->dominantDirection(flowLeft, leftB, lTotalP, lBoundingP, lDiffP);
        difftime("debug_fb_perf2: method:leftDominant", t1, debug_fb_perf2);
        this->lastRepupilDiffLeft.x +=  lTotalP.x;
        this->lastRepupilDiffLeft.y +=  lTotalP.y;
        //printf("L dominant F %u T %.3lf total %5.2lf %5.2lf\n", frameNum, timestamp, lTotalP.x, lTotalP.y);
    }
    if (canProceedR == true) {
        t1 = std::chrono::steady_clock::now();
        cv::calcOpticalFlowFarneback(this->pright, right, flowRight, 0.5, 3, 15, 3, 5, 1.2, 0);
        difftime("debug_fb_perf2: method:calcOpticalFlowFarnebackR", t1, debug_fb_perf2);
        t1 = std::chrono::steady_clock::now();
        int rightBw = this->rightRg.width*0.75, rightBh = this->rightRg.height*0.4;
        rightB = cv::Rect(this->rEye.x-(rightBw/2), this->rEye.y-(rightBh/2), rightBw, rightBh);
        this->dominantDirection(flowRight, rightB, rTotalP, rBoundingP, rDiffP);
        difftime("debug_fb_perf2: method:rightDominant", t1, debug_fb_perf2);
        this->lastRepupilDiffRight.x +=  rTotalP.x;
        this->lastRepupilDiffRight.y +=  rTotalP.y;
        //printf("R dominant F %u T %.3lf total %5.2lf %5.2lf\n", frameNum, timestamp, rTotalP.x, rTotalP.y);
    }

    // blink measure
    BlinkMeasureF bm(frameNum, timestamp, lDiffP, rDiffP, canProceedL, canProceedR, canUpdateL, canUpdateR);
    blinkMeasuref.push_back(bm);

    doLog(debug_fb_log_flow, "debug_fb_log_flow: F %u T %lf lTotal %5.2lf %5.2lf lbtotal %5.2lf %5.2lf lDiff %5.2lf %5.2lf rTotal %5.2lf %5.2lf rbtotal %5.2lf %5.2lf rDiff %5.2lf %5.2lf\n",
        frameNum, timestamp, lTotalP.x, lTotalP.y, lBoundingP.x, lBoundingP.y, lDiffP.x, lDiffP.y,
        rTotalP.x, rTotalP.y, rBoundingP.x, rBoundingP.y, rDiffP.x, rDiffP.y);
}
void Farneback::process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    std::chrono::time_point<std::chrono::steady_clock> t1;
    cv::Mat left, right, flowLeft, flowRight;
    cv::Rect leftB, rightB;

    if (flagReinit == true) {
        if (this->reinit(gray, left, right, timestamp, frameNum) != true) {
            doLog(debug_fb_log1, "debug_fb_log1: F %u T %lf reinit failed\n", frameNum, timestamp);
            return;
        } else {
            this->flagReinit = false;
        }
    } else {
        if ((frameNum % 2) == 0) {
            //return;
        }
        t1 = std::chrono::steady_clock::now();
        std::array<bool, 4> repupilRes = this->rePupil(gray, timestamp, frameNum);
        difftime("debug_fb_perf2: process:rePupil", t1, debug_fb_perf2);

        t1 = std::chrono::steady_clock::now();
        this->method(gray, repupilRes[0], repupilRes[1], repupilRes[2], repupilRes[4], left, right, flowLeft, flowRight, leftB, rightB, timestamp, frameNum);
        difftime("debug_fb_perf2: process:method", t1, debug_fb_perf2);

        if (debug_show_img_main == true) {
            t1 = std::chrono::steady_clock::now();
            this->drawOptFlowMap(this->leftRg, flowLeft, out, 5, cv::Scalar(0, 255, 0), 0);
            this->drawOptFlowMap(this->rightRg, flowRight, out, 5, cv::Scalar(0, 255, 0), 1);
            difftime("debug_fb_perf2: process:drawOptFlowMap", t1, debug_fb_perf2);
        }
    }

    if ((frameNum % 30) == 0) {
//        this->flagReinit = true;
    }

    // drawing and showing Mat out
    if (debug_show_img_main == true) {
        t1 = std::chrono::steady_clock::now();
        // draw pupil location
        circle(out, cv::Point(this->leftRg.x+this->lEye.x, this->leftRg.y+this->lEye.y), 3, cv::Scalar(0,255,0), -1, 8);
        circle(out, cv::Point(this->rightRg.x+this->rEye.x, this->rightRg.y+this->rEye.y), 3, cv::Scalar(0,255,0), -1, 8);

        // draw eyes bounding boxes
        cv::RNG rng(12345);
        cv::Scalar coolor = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
        cv::rectangle(out, cv::Rect(this->leftRg.x+leftB.x, this->leftRg.y+leftB.y, leftB.width, leftB.height), coolor, 1, 8, 0);
        cv::rectangle(out, cv::Rect(this->rightRg.x+rightB.x, this->rightRg.y+rightB.y, rightB.width, rightB.height), coolor, 1, 8, 0);
        difftime("debug_fb_perf2: process:drawing_out", t1, debug_fb_perf2);
    }
    if ((frameNum % 2) == 0 || true) {

        int rowsO = out.rows/4;
        int colsO = out.cols/4;
        int rows2 = out.rows/2;
        int cols2 = out.cols/2;

        cv::Rect mainZoomedRect = cv::Rect(colsO, rowsO, cols2, rows2);
        cv::Mat mainZoomedMat = out(mainZoomedRect);

        t1 = std::chrono::steady_clock::now();
        if (frameNum > 1) {
            imshowWrapper("leftR", this->pleft, debug_show_img_farne_eyes);
            imshowWrapper("rightR", this->pright, debug_show_img_farne_eyes);
        }
        imshowWrapper("left", left, debug_show_img_farne_eyes);
        imshowWrapper("right", right, debug_show_img_farne_eyes);
        imshowWrapper("main", out, debug_show_img_main);
        imshowWrapper("gray", mainZoomedMat, debug_show_img_gray);
        difftime("debug_fb_perf2: process:showimgs", t1, debug_fb_perf2);
    }

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

void Farneback::measureBlinks() {
    if (this->canCallMeasureBlinks || true) {
        while (blinkMeasuref.size() > 0) {
            BlinkMeasureF::measureBlinks(blinkMeasuref.front());
            blinkMeasuref.pop_front();
        }
    }
};

int Farneback::run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    std::chrono::time_point<std::chrono::steady_clock> t1;
    t1 = std::chrono::steady_clock::now();
    this->process(gray, out, timestamp, frameNum);
    difftime("debug_fb_perf1: process", t1, debug_fb_perf1);

    t1 = std::chrono::steady_clock::now();
    this->measureBlinks();
    difftime("debug_fb_perf1: measureBlinks", t1, debug_fb_perf1);

    //cv::swap(prevLeft, left);
    //cv::swap(prevRight, right);
    return 0;
};

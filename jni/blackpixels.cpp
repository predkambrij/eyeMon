
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
#include <blackpixels.hpp>




int Blackpixels::faceDetect(cv::Mat gray, cv::Rect *face) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
    if (faces.size() != 1) {
        return -1;
    }
    *face = faces[0];
    return 0;
}
void Blackpixels::eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point &leftPupil, cv::Point &rightPupil) {
    leftPupil  = findEyeCenter(faceROI, leftEyeRegion);
    rightPupil = findEyeCenter(faceROI, rightEyeRegion);
}
bool Blackpixels::preprocess(cv::Mat& left, cv::Mat& right, __attribute__((unused)) double timestamp, __attribute__((unused)) unsigned int frameNum) {
    GaussianBlur(left, left, cv::Size(3,3), 0);
    GaussianBlur(left, left, cv::Size(3,3), 0);
    cv::equalizeHist(left, left);
    cv::equalizeHist(right, right);
    return true;
}
bool Blackpixels::reinit(cv::Mat gray, cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum) {
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

    // preprocess only eye region(blur, eqHist)
    this->preprocess(left, right, timestamp, frameNum);
    left  = gray(this->leftRg).clone();
    right = gray(this->rightRg).clone();

    // locate and save pupil location
    this->eyeCenters(gray, this->leftRg, this->rightRg, this->lEye, this->rEye);
    this->initEyesDistance = (this->rightRg.x+this->rEye.x)-(this->leftRg.x+this->lEye.x);
    this->lLastTime = timestamp;
    this->rLastTime = timestamp;

    cv::Mat faceROI = gray(face);
    imshowWrapper("face", faceROI, debug_show_img_face);

    doLog(debug_fb_log1, "debug_fb_log1: F %u T %.3lf lEye %d %d rEye %d %d lLastTime %lf rLastTime %lf\n",
        frameNum, timestamp, this->lEye.x, this->lEye.y, this->rEye.x, this->rEye.y, this->lLastTime, this->rLastTime);

    return true;
}

std::array<bool, 4> Blackpixels::rePupil(cv::Mat gray, double timestamp, unsigned int frameNum) {
    std::chrono::time_point<std::chrono::steady_clock> t1;
    bool firePreprocess = false, canUpdateL = false, canUpdateR = false;
    cv::Point newLEyeLoc, newREyeLoc;
    unsigned int curXEyesDistance, curYEyesDistance;
    const int maxDiff = 15;
    int proceedDelay = 1;
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

};
double Blackpixels::countPixels(cv::Mat eye, cv::Rect bounding) {
    double totalGray=0;
    for(int y = 0; y < eye.rows; y += 1) {
        for(int x = 0; x < eye.cols; x += 1) {
            int grayPixel = eye.at<uchar>(y, x);
            if (x >= bounding.x && x < (bounding.x+bounding.width)
                && y >= bounding.y && y < (bounding.y+bounding.height)) {
                totalGray += grayPixel;
            }
        }
    }
    totalGray /= (bounding.width*bounding.height);
    return 255-totalGray;
}
void Blackpixels::method(cv::Mat gray, cv::Mat& left, cv::Mat& right, cv::Mat& tLeft, cv::Mat& tRight, cv::Rect& leftB, cv::Rect& rightB, double timestamp, unsigned int frameNum) {
    left = gray(this->leftRg);
    right = gray(this->rightRg);

    // preprocess only eye region(blur, eqHist)
    this->preprocess(left, right, timestamp, frameNum);
    left = left.clone();
    right = right.clone();

    cv::threshold(left, tLeft, 27, 255, CV_THRESH_BINARY);
    cv::threshold(right, tRight, 27, 255, CV_THRESH_BINARY);

    //cv::calcOpticalFlowFarneback(this->pleft, left, flowLeft, 0.5, 3, 15, 3, 5, 1.2, 0);
    //cv::calcOpticalFlowFarneback(this->pright, right, flowRight, 0.5, 3, 15, 3, 5, 1.2, 0);

    // bounding boxes
    int leftBw = this->leftRg.width*0.75, leftBh = this->leftRg.height*0.4;
    int rightBw = this->rightRg.width*0.75, rightBh = this->rightRg.height*0.4;
    leftB = cv::Rect(this->lEye.x-(leftBw/2), this->lEye.y-(leftBh/2), leftBw, leftBh);
    rightB = cv::Rect(this->rEye.x-(rightBw/2), this->rEye.y-(rightBh/2), rightBw, rightBh);
    double lNum = this->countPixels(tLeft, leftB);
    double rNum = this->countPixels(tRight, rightB);
    doLog(debug_bp_log_pix, "debug_bp_log_pix: F %u T %lf lNum %5.2lf rNum %5.2lf\n", frameNum, timestamp, lNum, rNum);
}
void Blackpixels::process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    cv::Mat left, right, tLeft, tRight;
    cv::Rect leftB, rightB;
    bool hasTLeftRight = false;

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
        this->rePupil(gray, timestamp, frameNum);
        this->method(gray, left, right, tLeft, tRight, leftB, rightB, timestamp, frameNum);
        hasTLeftRight = true;
    }

    if ((frameNum % 30) == 0) {
//        this->flagReinit = true;
    }

    // draw pupil location
    circle(out, cv::Point(this->leftRg.x+this->lEye.x, this->leftRg.y+this->lEye.y), 3, cv::Scalar(0,255,0), -1, 8);
    circle(out, cv::Point(this->rightRg.x+this->rEye.x, this->rightRg.y+this->rEye.y), 3, cv::Scalar(0,255,0), -1, 8);

    // draw eyes bounding boxes
    cv::RNG rng(12345);
    cv::Scalar coolor = cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
    cv::rectangle(out, cv::Rect(this->leftRg.x+leftB.x, this->leftRg.y+leftB.y, leftB.width, leftB.height), coolor, 1, 8, 0);
    cv::rectangle(out, cv::Rect(this->rightRg.x+rightB.x, this->rightRg.y+rightB.y, rightB.width, rightB.height), coolor, 1, 8, 0);

    if ((frameNum % 2) == 0||true) {
        if (this->hasPLeftRight == true) {
            imshowWrapper("leftR", this->pleft, debug_show_img_templ_eyes_tmpl);
            imshowWrapper("rightR", this->pright, debug_show_img_templ_eyes_tmpl);
        }
        if (hasTLeftRight == true) {
            imshowWrapper("leftSR", tLeft, debug_show_img_templ_eyes_tmpl);
            imshowWrapper("rightSR", tRight, debug_show_img_templ_eyes_tmpl);
        }
        imshowWrapper("left", left, debug_show_img_templ_eyes_tmpl);
        imshowWrapper("right", right, debug_show_img_templ_eyes_tmpl);
        imshowWrapper("main", out, debug_show_img_main);
        //imshowWrapper("gray", gray, debug_show_img_main);
    }

    this->hasPLeftRight = true;
    this->pleft = left;
    this->pright = right;
    return;
}

Blackpixels::Blackpixels () {
};

int Blackpixels::setup(const char* cascadeFileName) {
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
int Blackpixels::setJni(JNIEnv* jenv) {
};
#endif

int Blackpixels::run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    //cvtColor(rgb, grayx, COLOR_BGR2GRAY);
    //process(rgb, grayx, rgb);
    this->process(gray, out, timestamp, frameNum);

    //cv::swap(prevLeft, left);
    //cv::swap(prevRight, right);
    return 0;
};

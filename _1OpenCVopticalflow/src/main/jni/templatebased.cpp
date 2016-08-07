#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <common.hpp>
#include <blinkmeasure.hpp>
#include <templatebased.hpp>

TemplateBased::TemplateBased() {
    this->canProcess = false;
    this->frameNum = 0; this->frameNumt = 40;

    this->hasTemplate = false;

    this->lLastTime = -1; this->rLastTime = -1;
};

#ifdef IS_PHONE
void TemplateBased::setJni(JNIEnv* jenv) {
};
#endif

void TemplateBased::setup(const char* cascadeFileName) {
    try {
        if(!face_cascade.load(cascadeFileName)) {
            throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
        }
    } catch (const char* msg) {
        doLog(true, msg);
        throw;
    }
};
bool TemplateBased::preprocessing(cv::Mat& gray) {
    /*
    // light flash at the start
    this->frameNum++;
    if (this->canProcess == false) {
        if (this->frameNum >= this->frameNumt) {
            this->canProcess = true;
        } else {
            return false;
        }
    }*/
    std::chrono::time_point<std::chrono::steady_clock> t1;
    t1 = std::chrono::steady_clock::now();
    GaussianBlur(gray, gray, cv::Size(5,5), 0);
    difftime("GaussianBlur", t1, debug_tmpl_perf1);
    return true;
};

bool TemplateBased::eyesInit(cv::Mat& gray, double timestamp) {
    cv::Rect face;
    cv::Mat faceROI;
    std::chrono::time_point<std::chrono::steady_clock> t1;
    t1 = std::chrono::steady_clock::now();
    int fdRes = this->faceDetect(gray, &face);
    difftime("Face detect", t1, debug_tmpl_perf2);

    if (fdRes != 0) {
        return false;
    }

    faceROI = gray(face);
    imshowWrapper("face", faceROI, debug_show_img_face);

    int rowsO = (face).height/4.3;
    int colsO = (face).width/5.5;
    int rows2 = (face).height/4.3;
    int cols2 = (face).width/4.5;

    cv::Rect leftE(colsO, rowsO, cols2, rows2);
    cv::Rect rightE((face).width-colsO-rows2, rowsO, cols2, rows2);

    // mark where on the gray are eyes so we can define search region in the next frame
    // region size is double size of leftTemplate and leftTemplate size
    this->lEye = cv::Point(face.x+leftE.x, face.y+leftE.y);
    this->rEye = cv::Point(face.x+rightE.x, face.y+rightE.y);
    this->initialEyesDistance = this->rEye.x-this->lEye.x;
    this->lLastTime = timestamp;
    this->rLastTime = timestamp;
    doLog(debug_tmpl_log, "debug_tmpl_log: lEye %d %d rEye %d %d\n", lEye.x, lEye.y, rEye.x, rEye.y);


    cv::Mat left  = faceROI(leftE);
    cv::Mat right = faceROI(rightE);
// TODO - should wait for initial blink, so that eyes are open for sure
    left.copyTo(this->leftTemplate);
    right.copyTo(this->rightTemplate);

    cv::Mat lTemplSearch, rTemplSearch;
    cv::Rect lTemplSearchR, rTemplSearchR;

    bool res = this->updateTemplSearch(gray, lTemplSearchR, rTemplSearchR, lTemplSearch, rTemplSearch);
    if (res == false) {
        return false;
    }
    left.copyTo(this->leftTemplate);
    right.copyTo(this->rightTemplate);

    imshowWrapper("left", this->leftTemplate, debug_show_img_templ_eyes_tmpl);
    imshowWrapper("right", this->rightTemplate, debug_show_img_templ_eyes_tmpl);

    doLog(debug_tmpl_log, "debug_tmpl_log: reinit: timestamp %lf lLastTime %lf rLastTime %lf\n",
            timestamp, this->lLastTime, this->rLastTime);
    return true;
};
bool TemplateBased::updateTemplSearch(cv::Mat gray, cv::Rect& lTemplSearchR, cv::Rect& rTemplSearchR, cv::Mat& lTemplSearch, cv::Mat& rTemplSearch) {
// TODO take care that it won't get outside of gray
    int lTempColsHalf = this->leftTemplate.cols/2;
    int lTempRowsHalf = this->leftTemplate.rows/2;
    int rTempColsHalf = this->rightTemplate.cols/2;
    int rTempRowsHalf = this->rightTemplate.rows/2;
    int lTemplSearchSX = this->lEye.x-lTempColsHalf;
    int rTemplSearchSX = this->rEye.x-rTempColsHalf;
    int lTemplSearchSY = this->lEye.y-lTempRowsHalf;
    int rTemplSearchSY = this->rEye.y-rTempRowsHalf;
    int lTemplSearchEX = lTemplSearchSX+this->leftTemplate.cols*1.8;
    int lTemplSearchEY = lTemplSearchSY+this->leftTemplate.rows*2;
    int rTemplSearchEX = rTemplSearchSX+this->rightTemplate.cols*1.8;
    int rTemplSearchEY = rTemplSearchSY+this->rightTemplate.rows*2;
    if (lTemplSearchSX < 0 || lTemplSearchSY < 0 || rTemplSearchSX < 0 || rTemplSearchSY < 0
        || lTemplSearchEX > gray.cols || lTemplSearchEY > gray.rows || rTemplSearchEX > gray.cols || rTemplSearchEY > gray.rows) {
        this->hasTemplate = false;
        return false;
    }
    lTemplSearchR = cv::Rect(lTemplSearchSX, lTemplSearchSY, this->leftTemplate.cols*1.8, this->leftTemplate.rows*2);
    rTemplSearchR = cv::Rect(rTemplSearchSX, rTemplSearchSY, this->rightTemplate.cols*1.8, this->rightTemplate.rows*2);
    doLog(debug_tmpl_log, "debug_tmpl_log: AAA lTemplSearchR %d %d %d %d\n", lTemplSearchR.x, lTemplSearchR.y, lTemplSearchR.width, lTemplSearchR.height);
    doLog(debug_tmpl_log, "debug_tmpl_log: AAA rTemplSearchR %d %d %d %d\n", rTemplSearchR.x, rTemplSearchR.y, rTemplSearchR.width, rTemplSearchR.height);
    lTemplSearch = gray(lTemplSearchR);
    rTemplSearch = gray(rTemplSearchR);
    return true;
    //cv::equalizeHist(lTemplSearch, lTemplSearch);
    //cv::equalizeHist(rTemplSearch, rTemplSearch);
    //GaussianBlur(lTemplSearch, lTemplSearch, cv::Size(3,3), 0);
    //GaussianBlur(rTemplSearch, rTemplSearch, cv::Size(3,3), 0);
};
void TemplateBased::checkTracking(double timestamp, unsigned int frameNum) {
    if ((this->lLastTime+500) < timestamp || (this->rLastTime+500) < timestamp) {
        // we lost eyes, request reinit
        this->hasTemplate = false;
        doLog(debug_tmpl_log, "debug_tmpl_log: reinit: eyes were displaced timestamp %lf lLastTime %lf rLastTime %lf\n",
            timestamp, this->lLastTime, this->rLastTime);
    }
    int curXEyesDistance = this->rEye.x-this->lEye.x;
    int curYEyesDistance = abs(this->rEye.y-this->lEye.y);
    if (curXEyesDistance < (this->initialEyesDistance*0.75)
        || curXEyesDistance > (this->initialEyesDistance*1.30)
        || curYEyesDistance > this->initialEyesDistance*0.30) {
        doLog(debug_fb_log1, "debug_tmpl_log: F %u T %lf initEyesDistance %u curXEyesDistance %u curYEyesDistance %u\n",
            frameNum, timestamp, this->initialEyesDistance, curXEyesDistance, curYEyesDistance);
        this->hasTemplate = false;
    }
};
void TemplateBased::updateSearchRegion(cv::Point matchLocL, cv::Point matchLocR, double timestamp) {
    if ((abs(matchLocL.x-this->lEye.x) + abs(matchLocL.y-this->lEye.y)) < 20) {
        this->lEye.x = matchLocL.x;
        this->lEye.y = matchLocL.y;
        this->lLastTime = timestamp;
    }
    if ((abs(matchLocR.x-this->rEye.x) + abs(matchLocR.y-this->rEye.y)) < 20) {
        this->rEye.x = matchLocR.x;
        this->rEye.y = matchLocR.y;
        this->rLastTime = timestamp;
    }
};
void TemplateBased::method(cv::Mat& gray, cv::Mat& out, double timestamp, unsigned int frameNum) {
    std::chrono::time_point<std::chrono::steady_clock> t1;
    cv::Mat lTemplSearch, rTemplSearch, leftResult, rightResult;
    cv::Rect lTemplSearchR, rTemplSearchR;
    double minValL, maxValL, minValR, maxValR;
    cv::Point  minLocL, maxLocL, matchLocL, minLocR, maxLocR, matchLocR;

    imshowWrapper("left", this->leftTemplate, debug_show_img_templ_eyes_tmpl);
    imshowWrapper("right", this->rightTemplate, debug_show_img_templ_eyes_tmpl);

    // define template search region based on eyes' location in previous frame
    bool res = this->updateTemplSearch(gray, lTemplSearchR, rTemplSearchR, lTemplSearch, rTemplSearch);
    if (res == false) {
        return;
    }
    GaussianBlur(lTemplSearch, lTemplSearch, cv::Size(5,5), 0);
    GaussianBlur(rTemplSearch, rTemplSearch, cv::Size(5,5), 0);
    imshowWrapper("leftSR", lTemplSearch, debug_show_img_templ_eyes_cor);
    imshowWrapper("rightSR", rTemplSearch, debug_show_img_templ_eyes_cor);

// TODO we should estimate whether we lost eyes
// - location moved too far, correlation too low, (face not visible)
    t1 = std::chrono::steady_clock::now();
    cv::matchTemplate(lTemplSearch, this->leftTemplate, leftResult, CV_TM_SQDIFF_NORMED);
    cv::matchTemplate(rTemplSearch, this->rightTemplate, rightResult, CV_TM_SQDIFF_NORMED);
    difftime("debug_tmpl_perf2: matchTemplate (2x)", t1, debug_tmpl_perf2);
    imshowWrapper("leftR", leftResult, debug_show_img_templ_eyes_cor);
    imshowWrapper("rightR", rightResult, debug_show_img_templ_eyes_cor);

    //normalize(leftResult, leftResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    //normalize(rightResult, rightResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    minMaxLoc(leftResult, &minValL, &maxValL, &minLocL, &maxLocL, cv::Mat());
    minMaxLoc(rightResult, &minValR, &maxValR, &minLocR, &maxLocR, cv::Mat());
    double lcor = 1-minValL;
    double rcor = 1-minValR;
    doLog(debug_tmpl_log, "debug_tmpl_log: lcor %lf rcor %lf\n", lcor, rcor);

    // blink measure
    BlinkMeasure bm(frameNum, timestamp, lcor, rcor);
    blinkMeasure.push_back(bm);

    matchLocL = cv::Point(minLocL.x+lTemplSearchR.x, minLocL.y+lTemplSearchR.y);
    matchLocR = cv::Point(minLocR.x+rTemplSearchR.x, minLocR.y+rTemplSearchR.y);
    doLog(debug_tmpl_log, "debug_tmpl_log: debug matchLEye %d %d matchREye %d %d\n", matchLocL.x, matchLocL.y, matchLocR.x, matchLocR.y);
    doLog(debug_tmpl_log, "debug_tmpl_log: debug this->lEye %d %d this->rEye %d %d\n", this->lEye.x, this->lEye.y, this->lEye.x, this->lEye.y);

    // if eyes didn't move too much, update eyes position so search region can be re-centered the next time
    this->updateSearchRegion(matchLocL, matchLocR, timestamp);

    // re-init the tracking if we think that we lost eyes
    this->checkTracking(timestamp, frameNum);

    if (debug_show_img_main == true) {
        circle(out, cv::Point2f((float)matchLocL.x, (float)matchLocL.y), 10, cv::Scalar(0,255,0), -1, 8);
        rectangle(out, matchLocL, cv::Point(matchLocL.x+ leftTemplate.cols, matchLocL.y+leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);
        circle(out, cv::Point2f((float)matchLocR.x, (float)matchLocR.y), 10, cv::Scalar(0,255,0), -1, 8);
        rectangle(out, matchLocR, cv::Point(matchLocR.x+leftTemplate.cols, matchLocR.y+leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);

        rectangle(out, lTemplSearchR, CV_RGB(255, 255, 255), 0.5);
        rectangle(out, rTemplSearchR, CV_RGB(255, 255, 255), 0.5);
    }
};

void TemplateBased::process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    // we have template if we have template of open eyes
    if (this->hasTemplate == false) {
        if (!this->preprocessing(gray)) {
            // it will wait first 20 frames so that light flash ends
            // it blurs the grayscale image
            return;
        }
        this->prevTimestamp = timestamp;
        if (!this->eyesInit(gray, timestamp)) {
            // it finds eye region based on face detection and (TODO after first blink)
            imshowWrapper("main", out, debug_show_img_main);
            return;
        }

        this->hasTemplate = true;
    } else {
        // once we have eyes location we calculate correlation in the area around the eyes
        // if we believe that eyes are not in the search area anymore (person turned head or left the computer)
        //                                                                      we reinitialize (set hasTemplate to false)
        this->method(gray, out, timestamp, frameNum);
    }
    if (timestamp-this->prevTimestamp > 20000) {
        // reinitialize on regular intervals
        this->hasTemplate = false;
    }
};
void TemplateBased::flushMeasureBlinks() {
    BlinkMeasure::rewriteElementsToStateQueue(0, 0);
    BlinkMeasure::processStateMachineQueue();
    BlinkMeasure::lAdding = false;
    BlinkMeasure::rAdding = false;
    BlinkMeasure::joinBlinks();
}

int TemplateBased::measureBlinks(double curTimestamp) {
    int ret = 0;
    bool notifsCanProceed = false;
    while (blinkMeasure.size() > 0) {
        BlinkMeasure bm = blinkMeasure.front();
        blinkMeasure.pop_front();
        notifsCanProceed = BlinkMeasure::measureBlinks(bm);

        if (notifsCanProceed == true) {
            BlinkMeasure::processStateMachineQueue();
        }
    }

    bool wasBlink = BlinkMeasure::joinBlinks();
    if (wasBlink == true) {
        if (debug_blink_beeps == true) {
            system("/usr/bin/beep -l 200 &");
        }
    }
    bool n1UnderThreshold = false; // test (5 seconds)
    bool n2UnderThreshold = false; // 5 minutes
    bool n3UnderThreshold = false; // 20 minutes

    if (notifsCanProceed == true) {
        n1UnderThreshold = BlinkMeasure::checkN1Notifs(curTimestamp);
        if (n1UnderThreshold == true) {
            if (BlinkMeasure::n1UnderThreshold == false) {
                BlinkMeasure::n1UnderThreshold = true;
                if (debug_n1_beeps == true) {
                    system("/usr/bin/beep -r 10 -l 100 -f 800 &");
                }
            }
        } else {
            BlinkMeasure::n1UnderThreshold = false;
        }
    }

    // result bits q w e r
    // r - was blink
    ret += wasBlink?1:0;
    // e - n1 notif
    ret += n1UnderThreshold?2:0;
    // w - n1 notif
    ret += n2UnderThreshold?4:0;
    // q - n1 notif
    ret += n3UnderThreshold?8:0;

    return ret;
};
/*
void TemplateBased::checkNotificationStatus(double timestamp) {
    while (lBlinkChunks.size() > 0) {
        Blink b = lBlinkChunks.front();
        lBlinkChunks.pop_front();
        lBlinkTimeframeChunks.push_back(b);
        doLog(debug_notifications_log1, "NOTIFS: Moving left blink %lf\n", b.timestampStart);
    }
    while (rBlinkChunks.size() > 0) {
        Blink b = rBlinkChunks.front();
        rBlinkChunks.pop_front();
        rBlinkTimeframeChunks.push_back(b);
        doLog(debug_notifications_log1, "NOTIFS: Moving right blink %lf\n", b.timestampStart);
    }
    int timeFrameWindowLength = 60000;
    while (lBlinkTimeframeChunks.size() > 0) {
        Blink oldestB = lBlinkTimeframeChunks.front();
        if (oldestB.timestampStart > (timestamp - timeFrameWindowLength)) {
            break;
        } else {
            lBlinkTimeframeChunks.pop_front();
        }
    }
    while (rBlinkTimeframeChunks.size() > 0) {
        Blink oldestB = rBlinkTimeframeChunks.front();
        if (oldestB.timestampStart > (timestamp - timeFrameWindowLength)) {
            break;
        } else {
            rBlinkTimeframeChunks.pop_front();
        }
    }

};
void TemplateBased::frameTimeProcessing(double timestamp) {
    if (previousFrameTime == -1) {
        previousFrameTime = timestamp;
        return;
    }
    if (previousFrameTime < (timestamp-1000)) {
        Blink lb(previousFrameTime, timestamp, 1);
        lBlinkChunks.push_back(lb);
        Blink rb(previousFrameTime, timestamp, 1);
        rBlinkChunks.push_back(rb);
    }
    previousFrameTime = timestamp;
};*/
int TemplateBased::faceDetect(cv::Mat gray, cv::Rect *face) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
    if (faces.size() != 1) {
        return -1;
    }
    *face = faces[0];
    return 0;
};
int TemplateBased::run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    std::chrono::time_point<std::chrono::steady_clock> t1;
    std::chrono::time_point<std::chrono::steady_clock> ta;
    t1 = std::chrono::steady_clock::now();
    ta = std::chrono::steady_clock::now();
    //this->frameTimeProcessing(timestamp);
    //this->checkNotificationStatus(timestamp);
    difftime("-- frameTimeProcessing and checkNotificationStatus", t1, debug_tmpl_perf1);

    t1 = std::chrono::steady_clock::now();
    this->process(gray, out, timestamp, frameNum);
    difftime("-- process", t1, debug_tmpl_perf2);

    t1 = std::chrono::steady_clock::now();
    int result = this->measureBlinks(timestamp);
    difftime("-- measureBlinks", t1, debug_tmpl_perf2);
    if (frameNum % 2 == 0) {
        imshowWrapper("main", out, debug_show_img_main);
    }
    difftime("debug_tmpl_perfa:", ta, debug_tmpl_perfa);
    return result;
};

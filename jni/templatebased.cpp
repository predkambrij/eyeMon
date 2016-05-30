#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <common.hpp>

#include <blinkmeasure.hpp>

//using namespace cv;
//using namespace std;

class TemplateBased {
    cv::CascadeClassifier face_cascade;
    cv::Mat leftTemplate, rightTemplate;
    bool haveTemplate = false;
    int frameNum = 0;
    int frameNumt = 20;
    bool canProcess = false;
    int lEyex = -1, lEyey = -1;
    int rEyex = -1, rEyey = -1;
    int lLastTime = -1, rLastTime = -1;

    public: void setup(const char* cascadeFileName) {
        try {
            if(!face_cascade.load(cascadeFileName)) {
                throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
            }
        } catch (const char* msg) {
            doLog(true, msg);
            throw;
        }
    }

#ifdef IS_PHONE
    public: void setJni(JNIEnv* jenv) {
    }
#endif
    public: bool preprocessing(cv::Mat& gray) {
        // light flash at the start
        this->frameNum++;
        if (this->canProcess == false) {
            if (this->frameNum >= this->frameNumt) {
                this->canProcess = true;
            } else {
                return false;
            }
        }
        std::chrono::time_point<std::chrono::steady_clock> t1;
        t1 = std::chrono::steady_clock::now();
        GaussianBlur(gray, gray, cv::Size(5,5), 0);
        difftime("GaussianBlur", t1, debug_tmpl_perf1);
        return true;
    }

    public: bool eyesInit(cv::Mat& gray) {
        cv::Rect face;
        cv::Mat faceROI;
        std::chrono::time_point<std::chrono::steady_clock> t1;
        t1 = std::chrono::steady_clock::now();
        int fdRes = this->faceDetect(gray, &face);
        difftime("Face detect", t1, debug_tmpl_perf2);

        if (fdRes != 0) {
            return false;
        }

        faceROI = (gray)(face).clone();
        imshowWrapper("face", faceROI, debug_show_img_face);

        int rowsO = (face).height/4.3;
        int colsO = (face).width/5.5;
        int rows2 = (face).height/4.3;
        int cols2 = (face).width/4.5;

        cv::Rect leftE(colsO, rowsO, cols2, rows2);
        cv::Rect rightE((face).width-colsO-rows2, rowsO, cols2, rows2);

        cv::Mat left  = (faceROI)(leftE);
        cv::Mat right = (faceROI)(rightE);
        left.copyTo(this->leftTemplate);
        right.copyTo(this->rightTemplate);

        imshowWrapper("left", this->leftTemplate, debug_show_img_templ_eyes_tmpl);
        imshowWrapper("right", this->rightTemplate, debug_show_img_templ_eyes_tmpl);

        return true;
    }

    public: void process(cv::Mat gray, cv::Mat out, double timestamp) {
        std::chrono::time_point<std::chrono::steady_clock> t1;
        cv::Rect leftEyeRegion, rightEyeRegion;
        cv::Mat lTemplSearch, rTemplSearch;
        cv::Mat leftResult, rightResult;
        cv::Mat flowLeft, flowRight;

        if (!this->preprocessing(gray)) {
            // it will wait first 20 frames so that light flash ends
            return;
        }

        // we have template if we have template of open eyes
        if (this->haveTemplate == false) {
            if (!this->eyesInit(gray)) {
                // eyes initialization failed
                imshowWrapper("main", out, debug_show_img_main);
                return;
            }

            this->haveTemplate = true;
        } else {
            imshowWrapper("left", this->leftTemplate, debug_show_img_templ_eyes_tmpl);
            imshowWrapper("right", this->rightTemplate, debug_show_img_templ_eyes_tmpl);
            double minValL, maxValL, minValR, maxValR;
            cv::Point  minLocL, maxLocL, matchLocL, minLocR, maxLocR, matchLocR;

            t1 = std::chrono::steady_clock::now();
            cv::matchTemplate(gray, this->leftTemplate, leftResult, CV_TM_SQDIFF_NORMED);
            cv::matchTemplate(gray, this->rightTemplate, rightResult, CV_TM_SQDIFF_NORMED);
            difftime("matchTemplate (2x)", t1, debug_tmpl_perf2);

            imshowWrapper("leftR", leftResult, debug_show_img_templ_eyes_cor);
            imshowWrapper("rightR", rightResult, debug_show_img_templ_eyes_cor);

            //normalize(leftResult, leftResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
            //normalize(rightResult, rightResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
            //imshowWrapper("leftR1", leftResult);
            //imshowWrapper("rightR1", rightResult);
            minMaxLoc(leftResult, &minValL, &maxValL, &minLocL, &maxLocL, cv::Mat());
            minMaxLoc(rightResult, &minValR, &maxValR, &minLocR, &maxLocR, cv::Mat());
            double lcor = 1-minValL;
            double rcor = 1-minValR;
            doLog(debug_tmpl_log, "debug_tmpl_log: lcor %lf rcor %lf\n", lcor, rcor);

            // blink measure
            BlinkMeasure bm(timestamp, lcor, rcor);
            blinkMeasure.push_back(bm);

            if (debug_show_img_main == true) {
                matchLocL = minLocL;
                matchLocR = minLocR;

                circle(out, cv::Point2f((float)matchLocL.x, (float)matchLocL.y), 10, cv::Scalar(0,255,0), -1, 8);
                rectangle(out, matchLocL, cv::Point(matchLocL.x + leftTemplate.cols , matchLocL.y + leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);
                circle(out, cv::Point2f((float)matchLocR.x, (float)matchLocR.y), 10, cv::Scalar(0,255,0), -1, 8);
                rectangle(out, matchLocR, cv::Point(matchLocR.x + leftTemplate.cols , matchLocR.y + leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);
            }
        }
    }
    public: void measureBlinks() {
        BlinkMeasure::measureBlinks();
    }
    public: void checkNotificationStatus(double timestamp) {
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

    }
    public: void frameTimeProcessing(double timestamp) {
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
    }
    public: int faceDetect(cv::Mat gray, cv::Rect *face) {
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
        if (faces.size() != 1) {
            return -1;
        }
        *face = faces[0];
        return 0;
    }
    public: void run(cv::Mat gray, cv::Mat out, double timestamp) {
        std::chrono::time_point<std::chrono::steady_clock> t1;
        t1 = std::chrono::steady_clock::now();
        this->frameTimeProcessing(timestamp);
        this->checkNotificationStatus(timestamp);
        difftime("-- frameTimeProcessing and checkNotificationStatus", t1, debug_tmpl_perf1);

        t1 = std::chrono::steady_clock::now();
        this->process(gray, out, timestamp);
        difftime("-- process", t1, debug_tmpl_perf2);

        t1 = std::chrono::steady_clock::now();
        this->measureBlinks();
        difftime("-- measureBlinks", t1, debug_tmpl_perf2);

        imshowWrapper("main", out, debug_show_img_main);
    }

}; // end of TemplateBased class definition

#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <common.hpp>
#include <blinkmeasure.hpp>

using namespace cv;
using namespace std;

class TemplateBased {
    cv::CascadeClassifier face_cascade;
    Mat leftTemplate, rightTemplate;
    int haveTemplate = 0;

    public: int setup(const char* cascadeFileName) {
        try {
            if(!face_cascade.load(cascadeFileName)) {
                throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
            }
        } catch (const char* msg) {
            doLog(true, msg);
            throw;
        }
        return 0;
    }

#ifdef IS_PHONE
    public: int setJni(JNIEnv* jenv) {
    }
#endif

    public: int appendStatistics(double t, double lv, double rv) {
#ifndef IS_PHONE
        FILE * pFile;
        pFile = fopen("/home/developer/other/resources/statistics/statistics.txt","a");
        fprintf(pFile, "%lf\t%lf\t%lf\n", t, lv, rv);
        fclose(pFile);
#endif
    }

    public: int appendEmpty(double t) {
#ifndef IS_PHONE
        FILE * pFile;
        pFile = fopen("/home/developer/other/resources/statistics/statistics.txt","a");
        fprintf(pFile, "%lf\t\t\n", t);
        fclose(pFile);
#endif
    }

    public: int process(Mat gray, Mat out, double timestamp) {
        std::chrono::time_point<std::chrono::steady_clock> t1;
        cv::Rect face, leftEyeRegion, rightEyeRegion;
        Mat faceROI;
        Mat left, right;
        Mat leftResult, rightResult;
        Mat flowLeft, flowRight;

        t1 = std::chrono::steady_clock::now();
        GaussianBlur(gray, gray, Size(5,5), 0);
        difftime("GaussianBlur", t1, debug_tmpl_perf1);

        t1 = std::chrono::steady_clock::now();
        int fdRes = this->faceDetect(gray, &face);
        difftime("Face detect", t1, debug_tmpl_perf2);

        if (fdRes != 0) {
            this->appendEmpty(timestamp);
            imshowWrapper("main", out, debug_show_img_main);
            return -1;
        }

        if (this->haveTemplate == false) {
            imshowWrapper("face", faceROI, debug_show_img_face);
            faceROI = gray(face);

            int rowsO = face.height/4.3;
            int colsO = face.width/7;
            int rows2 = face.height/4;
            int cols2 = face.width/4;

            cv::Rect leftE(colsO, rowsO, cols2, rows2);
            cv::Rect rightE(face.width-colsO-rows2, rowsO, cols2, rows2);

            left  = faceROI(leftE);
            right = faceROI(rightE);
            left.copyTo(leftTemplate);
            right.copyTo(rightTemplate);

            imshowWrapper("left", leftTemplate, debug_show_img_templ_eyes_tmpl);
            imshowWrapper("right", rightTemplate, debug_show_img_templ_eyes_tmpl);

            this->haveTemplate = true;
        } else {
            double minValL, maxValL, minValR, maxValR;
            Point  minLocL, maxLocL, matchLocL, minLocR, maxLocR, matchLocR;

            t1 = std::chrono::steady_clock::now();
            cv::matchTemplate(gray, leftTemplate, leftResult, CV_TM_SQDIFF_NORMED);
            cv::matchTemplate(gray, rightTemplate, rightResult, CV_TM_SQDIFF_NORMED);
            difftime("matchTemplate (2x)", t1, debug_tmpl_perf2);

            imshowWrapper("leftR", leftResult, debug_show_img_templ_eyes_cor);
            imshowWrapper("rightR", rightResult, debug_show_img_templ_eyes_cor);

            //normalize(leftResult, leftResult, 0, 1, cv::NORM_MINMAX, -1, Mat());
            //normalize(rightResult, rightResult, 0, 1, cv::NORM_MINMAX, -1, Mat());
            //imshowWrapper("leftR1", leftResult);
            //imshowWrapper("rightR1", rightResult);
            minMaxLoc(leftResult, &minValL, &maxValL, &minLocL, &maxLocL, Mat());
            minMaxLoc(rightResult, &minValR, &maxValR, &minLocR, &maxLocR, Mat());
            double lcor = 1-minValL;
            double rcor = 1-minValR;
            doLog(debug_tmpl_log, "lcor %lf rcor %lf\n", lcor, rcor);

            // blink measure
            BlinkMeasure bm(timestamp, lcor, rcor);
            blinkMeasure.push_back(bm);

            // correlation log
            this->appendStatistics(timestamp, lcor, rcor);

            if (debug_show_img_main == true) {
                matchLocL = minLocL;
                matchLocR = minLocR;

                circle(out, Point2f((float)matchLocL.x, (float)matchLocL.y), 10, Scalar(0,255,0), -1, 8);
                rectangle(out, matchLocL, Point(matchLocL.x + leftTemplate.cols , matchLocL.y + leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);
                circle(out, Point2f((float)matchLocR.x, (float)matchLocR.y), 10, Scalar(0,255,0), -1, 8);
                rectangle(out, matchLocR, Point(matchLocR.x + leftTemplate.cols , matchLocR.y + leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);
            }
        }
    }
    public: int measureBlinks() {
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
    public: int faceDetect(Mat gray, cv::Rect *face) {
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
        if (faces.size() != 1) {
            return -1;
        }
        *face = faces[0];
        return 0;
    }
    public: int run(Mat gray, Mat out, double timestamp) {
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

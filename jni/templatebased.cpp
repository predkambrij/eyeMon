#include <opencv2/objdetect/objdetect.hpp>

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
            doLog(msg);
            throw;
        }
        return 0;
    }
#ifdef IS_PHONE
    public: int setJni(JNIEnv* jenv) {
    }
#endif
    public: int appendStatistics(double t, double lv, double rv) {
        FILE * pFile;
        pFile = fopen("/home/developer/other/resources/statistics/statistics.txt","a");
        fprintf(pFile, "%lf\t%lf\t%lf\n", t, lv, rv);
        fclose(pFile);
    }
    public: int appendEmpty(double t) {
        FILE * pFile;
        pFile = fopen("/home/developer/other/resources/statistics/statistics.txt","a");
        fprintf(pFile, "%lf\t\t\n", t);
        fclose(pFile);
    }
    public: int process(Mat gray, Mat out, double timestamp) {
        std::chrono::time_point<std::chrono::steady_clock> t1;
        cv::Rect face, leftEyeRegion, rightEyeRegion;
        Mat faceROI;
        Mat left, right;
        Mat leftResult, rightResult;
        Mat flowLeft, flowRight;

        t1 = std::chrono::steady_clock::now();
        //GaussianBlur(gray, gray, Size(5,5), 3.0);
        GaussianBlur(gray, gray, Size(5,5), 0);
        if (debug_tmpl_perf1 == true) {
            difftime("GaussianBlur", t1);
        }

        t1 = std::chrono::steady_clock::now();
        int fdRes = this->faceDetect(gray, &face);
        if (debug_tmpl_perf2 == true) {
            difftime("Face detect", t1);
        }

        if (fdRes != 0) {
            if (debug_show_img_main == true && PHONE == 0) {
                imshow("main", out);
            }
            this->appendEmpty(timestamp);
            return -1;
        }

        t1 = std::chrono::steady_clock::now();
        faceROI = gray(face);
        if (debug_tmpl_perf1 == true) {
            difftime("face gray", t1);
        }

        if (debug_show_img_face == true && PHONE == 0) {
            imshow("face", faceROI);
        }

        if (this->haveTemplate == false) {
            int rowsO = faceROI.rows/4.3;
            int colsO = faceROI.cols/7;
            int rows2 = faceROI.rows/4;
            int cols2 = faceROI.cols/4;

            cv::Rect leftE(colsO, rowsO, cols2, rows2);
            cv::Rect rightE(faceROI.cols-colsO-rows2, rowsO, cols2, rows2);

            left  = faceROI(leftE);
            right = faceROI(rightE);
            left.copyTo(leftTemplate);
            right.copyTo(rightTemplate);

            if (debug_show_img_templ_eyes_tmpl == true && PHONE == 0) {
                imshow("left", leftTemplate);
                imshow("right", rightTemplate);
            }
            this->haveTemplate = true;
        } else {
            double minValL, maxValL, minValR, maxValR;
            Point  minLocL, maxLocL, matchLocL, minLocR, maxLocR, matchLocR;
            t1 = std::chrono::steady_clock::now();
            matchTemplate(gray, leftTemplate, leftResult, CV_TM_SQDIFF_NORMED);
            matchTemplate(gray, rightTemplate, rightResult, CV_TM_SQDIFF_NORMED);
            if (debug_tmpl_perf2 == true) {
                difftime("matchTemplate (2x)", t1);
            }
            if (debug_show_img_templ_eyes_cor == true && PHONE == 0) {
                imshow("leftR", leftResult);
                imshow("rightR", rightResult);
            }
            //normalize(leftResult, leftResult, 0, 1, cv::NORM_MINMAX, -1, Mat());
            //normalize(rightResult, rightResult, 0, 1, cv::NORM_MINMAX, -1, Mat());
            //imshow("leftR1", leftResult);
            //imshow("rightR1", rightResult);
            minMaxLoc(leftResult, &minValL, &maxValL, &minLocL, &maxLocL, Mat());
            minMaxLoc(rightResult, &minValR, &maxValR, &minLocR, &maxLocR, Mat());
            double lcor = 1-minValL;
            double rcor = 1-minValR;
            if (debug_tmpl_log == true) {
                printf("lcor %lf rcor %lf\n", lcor, rcor);
            }

            BlinkMeasure bm(timestamp, lcor, rcor);
            blinkMeasure.push_back(bm);

            this->appendStatistics(timestamp, lcor, rcor);
            //printf("lcor %lf rcor %lf\n", maxValL, maxValR);
            //cout << minLocL << endl;
            matchLocL = minLocL;
            matchLocR = minLocR;
            if (debug_show_img_main == true) {
                circle(out, Point2f((float)matchLocL.x, (float)matchLocL.y), 10, Scalar(0,255,0), -1, 8);
                rectangle(out, matchLocL, Point(matchLocL.x + leftTemplate.cols , matchLocL.y + leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);
                circle(out, Point2f((float)matchLocR.x, (float)matchLocR.y), 10, Scalar(0,255,0), -1, 8);
                rectangle(out, matchLocR, Point(matchLocR.x + leftTemplate.cols , matchLocR.y + leftTemplate.rows), CV_RGB(255, 255, 255), 0.5);
                //printf("lcor %lf rcor %lf\n", maxValL, maxValR);
            }
        }
    }
    public: int measureBlinks() {
        BlinkMeasure::measureBlinks();
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
        this->process(gray, out, timestamp);
        if (debug_tmpl_perf2 == true) {
            difftime("-- process", t1);
        }
        t1 = std::chrono::steady_clock::now();
        this->measureBlinks();
        if (debug_tmpl_perf2 == true) {
            difftime("-- measureBlinks", t1);
        }
#ifndef IS_PHONE
        if (debug_show_img_main == true && PHONE == 0) {
            imshow("main", out);
        }
#endif
    }

}; // end of TemplateBased class definition

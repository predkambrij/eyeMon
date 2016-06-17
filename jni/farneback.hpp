#ifndef FARNEBACK_H
#define FARNEBACK_H

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>


void fbDrawOptFlowMap (cv::Rect face, cv::Rect eyeE, const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye);

class Farneback {
    public:
        cv::Point lEye, rEye;
        double lLastTime, rLastTime;
        cv::Mat pgray;
        
        //cv::Rect face, leftEyeRegion, rightEyeRegion;
        cv::Rect faceRef, leftE, rightE;
        bool flagReinit = true, flagPupilSearch = false;

        Farneback();
        int faceDetect(cv::Mat gray, cv::Rect *face);
        void eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point& leftPupil, cv::Point& rightPupil);
        void method(cv::Mat gray, cv::Mat& faceROI, cv::Mat& left, cv::Mat& right, cv::Mat& flowLeft, cv::Mat& flowRight);
        void rePupil();
        void updateSearch(cv::Mat gray, cv::Rect& lTemplSearchR, cv::Rect& rTemplSearchR, cv::Mat& lTemplSearch, cv::Mat& rTemplSearch);
        bool reinit(cv::Mat gray, cv::Mat& faceROI, cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum);
        int run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
        int setup(const char* cascadeFileName);
        bool preprocess(cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum);
        void process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
#ifdef IS_PHONE
        int setJni(JNIEnv* jenv);
#endif
};

#endif

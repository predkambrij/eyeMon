#ifndef FARNEBACK_H
#define FARNEBACK_H

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>




class Farneback {
    public:
        cv::Point lEye, rEye;
        double lLastTime, rLastTime;
        cv::Mat pleft, pright;
        cv::Rect leftRg, rightRg;
        
        //cv::Rect face, leftEyeRegion, rightEyeRegion;
        bool flagReinit = true, flagPupilSearch = false;

        Farneback();
        int faceDetect(cv::Mat gray, cv::Rect *face);
        void eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point& leftPupil, cv::Point& rightPupil);
        void method(cv::Mat gray, cv::Mat& left, cv::Mat& right, cv::Mat& flowLeft, cv::Mat& flowRight, cv::Rect& leftB, cv::Rect& rightB, double timestamp, unsigned int frameNum);
        void rePupil(cv::Mat gray, double timestamp, unsigned int frameNum);
        void dominantDirection(cv::Mat flow, cv::Point& updateLoc);
        void updateSearch(cv::Mat gray, cv::Rect& lTemplSearchR, cv::Rect& rTemplSearchR, cv::Mat& lTemplSearch, cv::Mat& rTemplSearch);
        bool reinit(cv::Mat gray, cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum);
        int run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
        int setup(const char* cascadeFileName);
        bool preprocess(cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum);
        void process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
        void drawOptFlowMap (cv::Rect eyeE, const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye);
#ifdef IS_PHONE
        int setJni(JNIEnv* jenv);
#endif
};

#endif

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
        bool useDoubleLengthDiff = false;
        bool hasPLeftRight = false;
        bool canCallMeasureBlinks = true;
        bool onlyLower = false;
        cv::Point lEye, rEye;
        double lLastTime, rLastTime;
        double lastRepupilTime = 0;
        cv::Point2d lastRepupilDiffLeft, lastRepupilDiffRight;
        unsigned int initEyesDistance;
        cv::Mat pleft, pright, ppleft, ppright;
        cv::Rect leftRg, rightRg;
        
        //cv::Rect face, leftEyeRegion, rightEyeRegion;
        bool flagReinit = true, flagPupilSearch = false;

        Farneback();
        int faceDetect(cv::Mat gray, cv::Rect *face);
        void eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point& leftPupil, cv::Point& rightPupil, double timestamp, unsigned int frameNum);
        void method(cv::Mat gray, bool canProceedL, bool canProceedR, bool canUpdateL, bool canUpdateR, cv::Mat& left, cv::Mat& right, cv::Mat& flowLeft, cv::Mat& flowRight, cv::Rect& leftB, cv::Rect& rightB, double timestamp, unsigned int frameNum);
        std::array<bool, 4> rePupil(cv::Mat gray, double timestamp, unsigned int frameNum);
        void dominantDirection(cv::Mat flow, cv::Rect bounding, cv::Point2d& totalP, cv::Point2d& boundingP, cv::Point2d& diffP);
        bool reinit(cv::Mat gray, cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum);
        int run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
        int setup(const char* cascadeFileName);
        bool preprocess(cv::Mat& left, cv::Mat& right, double timestamp, unsigned int frameNum);
        void process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
        int measureBlinks();
        void drawOptFlowMap (cv::Rect eyeE, const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye);
#ifdef IS_PHONE
        int setJni(JNIEnv* jenv);
#endif
};

#endif

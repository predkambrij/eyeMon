#ifndef OPTFLOW_H
#define OPTFLOW_H

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>


void drawOptFlowMap (const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye);
void getLeftRightEyeMat(cv::Mat gray, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Mat *left, cv::Mat *right);
int faceDetect(cv::Mat gray, cv::Rect *face);
void eyeRegions(cv::Rect face, cv::Rect *leftEyeRegion, cv::Rect *rightEyeRegion);
void eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point& leftPupil, cv::Point& rightPupil);
void showResult(cv::Mat cflow, cv::Rect face, cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point leftPupil, cv::Point rightPupil);


class OptFlow {
    public:
        int num = 0;
        int flg1 = 0;
        int resetDelay = 0;
        cv::Mat pgray;
        cv::Mat oprev, opprev;
        cv::Mat loprev, lopprev;
        cv::Mat roprev, ropprev;
        
        cv::Rect face, leftEyeRegion, rightEyeRegion;
        cv::Rect leftE, rightE;

        cv::Mat rgb, grayx, left, right, prevLeft, prevRight;
        unsigned long long int ns = 0;
        cv::vector<cv::Point2f> lpoints[2], rpoints[2];
        const int MAX_COUNT = 501;
        cv::TermCriteria termcrit;
        cv::Size subPixWinSize, winSize;
        bool yes = false;
        bool yes1 = false;
        cv::Point2f point;
        OptFlow();
        int run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
        int setup(const char* cascadeFileName);
        void process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
#ifdef IS_PHONE
        int setJni(JNIEnv* jenv);
#endif
};

#endif

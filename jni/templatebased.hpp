#ifndef TEMPLATEBASED_H
#define TEMPLATEBASED_H

#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#ifdef IS_PHONE
#include <jni.h>
#endif

class TemplateBased {
    public:
        cv::CascadeClassifier face_cascade;
        bool canProcess;
        int frameNum, frameNumt;
        cv::Mat leftTemplate, rightTemplate;
        bool hasTemplate;
        double prevTimestamp;
        double initialEyesDistance;
        cv::Point lEye, rEye;
        double lLastTime, rLastTime;

        TemplateBased();
        void setup(const char* cascadeFileName);
        bool preprocessing(cv::Mat& gray);
        bool eyesInit(cv::Mat& gray, double timestamp);
        bool updateTemplSearch(cv::Mat gray, cv::Rect& lTemplSearchR, cv::Rect& rTemplSearchR, cv::Mat& lTemplSearch, cv::Mat& rTemplSearch);
        void method(cv::Mat& gray, cv::Mat& out, double timestamp, unsigned int frameNum);
        void process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);
        void flushMeasureBlinks();
        int measureBlinks(double curTimestamp);
        //void checkNotificationStatus(double timestamp);
        //void frameTimeProcessing(double timestamp);
        int faceDetect(cv::Mat gray, cv::Rect *face);
        void updateSearchRegion(cv::Point matchLocL, cv::Point matchLocR, double timestamp);
        void checkTracking(double timestamp, unsigned int frameNum);
        int run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum);

#ifdef IS_PHONE
        void setJni(JNIEnv* jenv);
#endif

};


#endif

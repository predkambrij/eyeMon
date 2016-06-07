#ifndef TEMPLATEBASED_H
#define TEMPLATEBASED_H

#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/imgproc/imgproc.hpp"

class TemplateBased {
    public:
        cv::CascadeClassifier face_cascade;
        bool canProcess;
        int frameNum, frameNumt;
        cv::Mat leftTemplate, rightTemplate;
        bool hasTemplate;
        cv::Point lEye, rEye;
        int lLastTime, rLastTime;

        TemplateBased();
        void setup(const char* cascadeFileName);
        bool preprocessing(cv::Mat& gray);
        bool eyesInit(cv::Mat& gray, double timestamp);
        void updateTemplSearch(cv::Mat gray, cv::Rect& lTemplSearchR, cv::Rect& rTemplSearchR, cv::Mat& lTemplSearch, cv::Mat& rTemplSearch);
        void method(cv::Mat& gray, cv::Mat& out, double timestamp);
        void process(cv::Mat gray, cv::Mat out, double timestamp);
        void measureBlinks();
        void checkNotificationStatus(double timestamp);
        void frameTimeProcessing(double timestamp);
        int faceDetect(cv::Mat gray, cv::Rect *face);
        void updateSearchRegion(cv::Point matchLocL, cv::Point matchLocR, double timestamp);
        void checkTracking(double timestamp);
        void run(cv::Mat gray, cv::Mat out, double timestamp);

#ifdef IS_PHONE
        void setJni(JNIEnv* jenv);
#endif

};


#endif

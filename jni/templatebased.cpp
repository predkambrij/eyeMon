#include <opencv2/objdetect/objdetect.hpp>

#include <common.hpp>

using namespace cv;
using namespace std;

class TemplateBased {
    cv::CascadeClassifier face_cascade;

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
    public: int process(Mat gray, Mat out) {
        std::chrono::time_point<std::chrono::steady_clock> t1;
        cv::Rect face, leftEyeRegion, rightEyeRegion;
        Mat faceROI;
        Mat left, right;
        Mat flowLeft, flowRight;

        t1 = std::chrono::steady_clock::now();
        if (this->faceDetect(gray, &face) != 0) {
            if (debug_show_img == true && PHONE == 0) {
                imshow("main", out);
            }
            return -1;
        }
        difftime("Face detect", t1);

        if (debug_show_img_face == true && PHONE == 0) {
            faceROI = gray(face);
            imshow("face", faceROI);
        }
        int rowsO = faceROI.rows/5;
        int colsO = faceROI.cols/5;
        int rows2 = faceROI.rows/3;
        int cols2 = faceROI.cols/3;

        cv::Rect leftE(colsO, rowsO, cols2, rows2);
        cv::Rect rightE(faceROI.cols-colsO-rows2, rowsO, cols2, rows2);

        left = faceROI(leftE);
        right = faceROI(rightE);

        imshow("left", left);
        imshow("right", right);

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
    public: int run(Mat gray, Mat out) {
        this->process(gray, out);
#ifndef IS_PHONE
        if (debug_show_img == true && PHONE == 0) {
            imshow("main", out);
        }
#endif
    }

}; // end of TemplateBased class definition

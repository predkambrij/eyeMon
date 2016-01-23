#include <stdio.h>
#include <iostream>
#include <ctime>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

#include <eyelike/main.cpp>

using namespace cv;
using namespace std;

#define DEBUG_LEVEL 3
//#define PHONE 0

//JNIEnv* env;
int PHONE = 1, farne = 0;
void doLogClock(const char* format, const char* title, double diffms) {
    if (PHONE != 1) {
        printf(format, title, diffms);
        printf("\n");
        return;
    }
}
void doLog(const char* text) {
    if (PHONE != 1) {
        printf("%s\n", text);
        return;
    }
    /*
    // malloc room for the resulting string
    char *szResult;
    szResult = (char*)malloc(sizeof(char)*500);

    auto end = std::chrono::high_resolution_clock::now();
    unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

    // standard sprintf
    sprintf(szResult, "res: %llu %s", ns, text);// szFormat sum CV_VERSION

    // get an object string
    jstring result = (env)->NewStringUTF(szResult);

    __android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, "NDK:OPTFLOF: [%s]", szResult);// szLogThis

    // cleanup
    free(szResult);*/
}

void diffclock(char const *title, clock_t clock2) {
    clock_t clock1 = clock();
    double diffticks = 1000.0 * (clock1 - clock2);
    double diffms    = diffticks / CLOCKS_PER_SEC;

    doLogClock("%s: %f", title, diffms);
}
int leftXOffset=100, leftYOffset=170, leftCols=100, leftRows=100;
int rightXOffset=250, rightYOffset=170, rightCols=100, rightRows=100;

void drawOptFlowMap (const Mat flow, Mat cflowmap, int step, const Scalar& color, int eye) {
    //circle(cflowmap, Point2f((float)10, (float)10), 3, Scalar(0,255,0), -1, 8);
    circle(cflowmap, Point2f((float)15, (float)15), 10, Scalar(0,255,0), -1, 8);
    int xo, yo;
    if (eye == 0) {
        xo = leftXOffset;
        yo = leftYOffset;
    } else {
        xo = rightXOffset;
        yo = rightYOffset;
    }
    for(int y = 0; y < flow.rows; y += step) {
        for(int x = 0; x < flow.cols; x += step) {
            
            const Point2f& fxy = flow.at< Point2f>(y, x);
            int px = x+xo, py = y+yo;
            line(cflowmap, Point(px,py), Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), color);
            //circle(cflowmap, Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), 1, color, -1);
            circle(cflowmap, Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), 1, color, -1, 8);

            //circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
        }
    }
}

int preprocess(Mat frame, Mat *gray, Mat *left, Mat *right) {
    int resizeFactor = 1;
    //resize(frame, *output, Size(frame.size().width/resizeFactor, frame.size().height/resizeFactor));

    if (PHONE == 0) {
      //cvtColor(frame, *gray, CV_BGR2GRAY);
      std::vector<cv::Mat> rgbChannels(3);
      cv::split(frame, rgbChannels);
      *gray = rgbChannels[2];
    }

    (*gray)(cv::Rect(leftXOffset,leftYOffset,leftCols,leftRows)).copyTo(*left);
    (*gray)(cv::Rect(rightXOffset,rightYOffset,rightCols,rightRows)).copyTo(*right);
}

int process1(Mat pleft, Mat left, Mat pright, Mat right, Mat cflow) {
    clock_t start;
    Mat flow, flow1;

    start = clock();
    calcOpticalFlowFarneback(pleft, left, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
    calcOpticalFlowFarneback(pright, right, flow1, 0.5, 3, 15, 3, 5, 1.2, 0);
    diffclock("- farneback", start);

    start = clock();
    diffclock("- cvtColor", start);

    start = clock();
    drawOptFlowMap(flow, cflow, 10, Scalar(0, 255, 0), 0);
    drawOptFlowMap(flow1, cflow, 10, Scalar(0, 255, 0), 1);
    
    //drawOptFlowMap(flow, *cflow, 10, CV_RGB(0, 255, 0));
    diffclock("- drawOptFlowMap", start);

    if (DEBUG_LEVEL >= 1 && PHONE == 0) {
        imshow("OpticalFlowFarneback", cflow);
    }

    if (DEBUG_LEVEL >= 2 && PHONE == 0) {
        imshow("left", left);
        imshow("right", right);
    }
}

int faceDetect(Mat gray, cv::Rect *face, Mat *faceROI) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
    if (faces.size() != 1) {
        return -1;
    }
    *face = faces[0];
    *faceROI = gray(*face);
    if (kSmoothFaceImage) {
        double sigma = kSmoothFaceFactor * (*face).width;
        GaussianBlur(*faceROI, *faceROI, cv::Size(0, 0), sigma);
    }
    return 0;
}

int eyeRegions(cv::Rect face, cv::Rect *leftEyeRegion, cv::Rect *rightEyeRegion) {
    int eye_region_width = face.width * (kEyePercentWidth/100.0);
    int eye_region_height = face.width * (kEyePercentHeight/100.0);
    int eye_region_top = face.height * (kEyePercentTop/100.0);
    (*leftEyeRegion) = cv::Rect(face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
    (*rightEyeRegion) = cv::Rect(face.width - eye_region_width - face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
}
int eyeCenters(Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point *leftPupil, cv::Point *rightPupil) {
    *leftPupil  = findEyeCenter(faceROI, leftEyeRegion, "Left Eye");
    *rightPupil = findEyeCenter(faceROI, rightEyeRegion, "Right Eye");
}

int showResult(Mat cflow, cv::Rect face, Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point leftPupil, cv::Point rightPupil) {
    // change eye centers to face coordinates
    rightPupil.x += rightEyeRegion.x; rightPupil.y += rightEyeRegion.y;
    leftPupil.x += leftEyeRegion.x; leftPupil.y += leftEyeRegion.y;
    rightPupil.x += face.x; rightPupil.y += face.y;
    leftPupil.x += face.x; leftPupil.y += face.y;

    circle(cflow, Point2f((float)15, (float)15), 10, Scalar(0,255,0), -1, 8);
    circle(cflow, rightPupil, 3, Scalar(0,255,0), -1, 8);
    circle(cflow, leftPupil, 3, Scalar(0,255,0), -1, 8);

    // draw eye centers
    if (PHONE == 0) {
        imshow("main", cflow);
    }
}

int setUp(const char* cascadePath) {
    // throw;
    try {
        if(!face_cascade.load(cascadePath)) {
            throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
        }
    } catch (const char* msg) {
        doLog(msg);
        throw;
    }
}
int process(Mat frame, Mat gray, Mat pleft, Mat left, Mat pright, Mat right, Mat cflow) {
    cv::Rect face, leftEyeRegion, rightEyeRegion;
    cv::Point leftPupil, rightPupil;
    Mat faceROI;

    if (farne == 0) {
        if (faceDetect(gray, &face, &faceROI) != 0) {
            return -1;
        }
        eyeRegions(face, &leftEyeRegion, &rightEyeRegion);
        eyeCenters(faceROI, leftEyeRegion, rightEyeRegion, &leftPupil, &rightPupil);
        showResult(frame, face, faceROI, leftEyeRegion, rightEyeRegion, leftPupil, rightPupil);
    } else {
        process1(pleft, left, pright, right, frame);
    }
}

int main() {
    PHONE = 0; farne = 0;
    // video source
    // char fileName[100] = "/home/developer/other/posnetki/o4_29.mp4";
    char fileName[200] = "/home/developer/other/android_deps/OpenCV-2.4.10-android-sdk/samples/optical-flow/res/raw/lbpcascade_frontalface.xml";
    //char fileName[100] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
    // VideoCapture stream1(fileName);   //0 is the id of video device.0 if you have only one camera   
    VideoCapture stream1(0);   //0 is the id of video device.0 if you have only one camera   

    // controls
    int pause = 0, firstLoop = 1;
    clock_t loopStart, start;
    Mat frame, gray,  pleft, left,  pright, right,  cflow;

    setUp(fileName);

    if (farne == 0) {
        cv::namedWindow(face_window_name,CV_WINDOW_NORMAL); cv::moveWindow(face_window_name, 10, 100);
        cv::namedWindow("main",CV_WINDOW_NORMAL); cv::moveWindow("main", 10, 100);
        // cv::namedWindow("Right Eye",CV_WINDOW_NORMAL); cv::moveWindow("Right Eye", 10, 600);
        // cv::namedWindow("Left Eye",CV_WINDOW_NORMAL); cv::moveWindow("Left Eye", 10, 800);
        // createCornerKernels(), at the end // releaseCornerKernels(); // ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2), 43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);
    }

    while (true) { cv::flip(frame, frame, 1);
        if(!(stream1.read(frame))) {
            doLog(" --(!) No captured frame -- Break!");
            return 0;
        }

        if (firstLoop == 1) {
            loopStart = clock();
            preprocess(frame, &gray, &pleft, &pright);
            firstLoop = 0;
            continue;
        }

        diffclock("\nwhole loop", loopStart);
        loopStart = clock();

        start = clock();
        preprocess(frame, &gray, &left, &right);
        diffclock("preprocess", start);

        start = clock();
        process(frame, gray, pleft, left, pright, right, frame);
        diffclock("process", start);

        // flow control
        int c = cv::waitKey(10);
        if(c == 27) {
            // esc
            break;
        } else if((char)c == 'p') {
            pause = 1;
        } else if((char)c == 'f') {
            imwrite("/tmp/frame.png",cflow);
        } else if (pause == 1) {
            while (true) {
                int c = cv::waitKey(10);
                if((char)c == 'p') {
                    pause = 0;
                    break;
                } else if((char)c == 'n') {
                    break;
                }
                //sleep(100)
            }
        }

        pleft = left.clone(); pright = right.clone();
    }

    return 0;
}

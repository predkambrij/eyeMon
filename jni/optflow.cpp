#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <android/log.h>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>
#include <chrono>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

#include <eyelike/main.cpp>

#include <common.hpp>

using namespace cv;
using namespace std;

//// farneb code
#define DEBUG_LEVEL 3
//#define PHONE 0

//JNIEnv* env;
int farne = 0;
void doLogClock(const char* format, const char* title, double diffms) {
    if (PHONE != 1) {
        printf(format, title, diffms);
        printf("\n");
        return;
    }
}

void diffclock(char const *title, clock_t clock2) {
    clock_t clock1 = clock();
    double diffticks = 1000.0 * (clock1 - clock2);
    double diffms    = diffticks / CLOCKS_PER_SEC;

    doLogClock("%s: %f", title, diffms);
}
int leftXOffset=200, leftYOffset=130, leftCols=100, leftRows=100;
int rightXOffset=350, rightYOffset=130, rightCols=100, rightRows=100;

int leftXp1=0, leftYp1=0, rightXp1=0, rightYp1=0;
int leftXlast=0, leftYlast=0, rightXlast=0, rightYlast=0;
int leftXavg=0, leftYavg=0, rightXavg=0, rightYavg=0;
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
            printf("%.0f ", fxy.y);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

int getGray(Mat frame, Mat *gray) {
    // int resizeFactor = 1;
    //resize(frame, *output, Size(frame.size().width/resizeFactor, frame.size().height/resizeFactor));
    //cvtColor(frame, *gray, CV_BGR2GRAY);
    std::vector<cv::Mat> rgbChannels(3);
    cv::split(frame, rgbChannels);
    *gray = rgbChannels[2];
}

int eye_region_width, eye_region_height;

int getLeftRightEyeMat(Mat gray, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, Mat *left, Mat *right) {
    // printf("eye_region_width %d, eye_region_height %d\n", eye_region_width, eye_region_height);
    // printf("leftEyeRegion %d, leftEyeRegion %d\n", leftEyeRegion.x, leftEyeRegion.y);
    // printf("rightEyeRegion %d, rightEyeRegion %d\n", rightEyeRegion.x, rightEyeRegion.y);
    // gray(cv::Rect(leftEyeRegion.x, leftEyeRegion.y, 60, 70)).copyTo(*left);
    // gray(cv::Rect(rightEyeRegion.x, rightEyeRegion.y, 60, 70)).copyTo(*right);
    gray(cv::Rect(leftXOffset, leftYOffset, leftCols, leftRows)).copyTo(*left);
    gray(cv::Rect(rightXOffset, rightYOffset, rightCols, rightRows)).copyTo(*right);
}

int faceDetect(Mat gray, cv::Rect *face) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
    if (faces.size() != 1) {
        return -1;
    }
    *face = faces[0];
    return 0;
}
int eyeRegions(cv::Rect face, cv::Rect *leftEyeRegion, cv::Rect *rightEyeRegion) {
    eye_region_width = face.width * (kEyePercentWidth/100.0);
    eye_region_height = face.width * (kEyePercentHeight/100.0);
    int eye_region_top = face.height * (kEyePercentTop/100.0);
    (*leftEyeRegion) = cv::Rect(face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
    (*rightEyeRegion) = cv::Rect(face.width - eye_region_width - face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
}
int eyeCenters(Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point *leftPupil, cv::Point *rightPupil) {
    *leftPupil  = findEyeCenter(faceROI, leftEyeRegion, "Left Eye");
    *rightPupil = findEyeCenter(faceROI, rightEyeRegion, "Right Eye");
}

int showResult(Mat cflow, cv::Rect face, Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point leftPupil, cv::Point rightPupil) {
    // // change eye centers to face coordinates
    // rightPupil.x += rightEyeRegion.x; rightPupil.y += rightEyeRegion.y;
    // leftPupil.x += leftEyeRegion.x; leftPupil.y += leftEyeRegion.y;
    // rightPupil.x += face.x; rightPupil.y += face.y;
    // leftPupil.x += face.x; leftPupil.y += face.y;
    // // leftXOffset = leftPupil.x - 50; rightXOffset = rightPupil.x - 50;
    // // leftYOffset = leftPupil.y - 50; rightYOffset = rightPupil.y - 50;

    // // if (leftXavg==0) {
    // //     // first iteration
    // //     leftXp1=leftPupil.x; leftXlast=leftPupil.x; leftXavg=leftPupil.x;
    // //     leftYp1=leftPupil.y; leftYlast=leftPupil.y; leftYavg=leftPupil.y;
    // //     rightXp1=rightPupil.x; rightXlast=rightPupil.x; rightXavg=rightPupil.x;
    // //     rightYp1=rightPupil.y; rightYlast=rightPupil.y; rightYavg=rightPupil.y;
    // // } else {
    // //     // calculate avg
    // //     leftXavg=(leftXp1+leftXlast+leftPupil.x)/3; leftYavg=(leftYp1+leftYlast+leftPupil.y)/3;
    // //     rightXavg=(rightXp1+rightXlast+rightPupil.x)/3; rightYavg=(rightYp1+rightYlast+rightPupil.y)/3;
    // //     // rotate
    // //     leftXp1=leftXlast; rightXp1=rightXlast; leftYp1=leftYlast; rightYp1=rightYlast;
    // //     leftXlast=leftPupil.x; rightXlast=rightPupil.x; leftYlast=leftPupil.y; rightYlast=rightPupil.y;
    // // }
    // // leftXOffset = leftXavg-50; leftYOffset = leftYavg-50;
    // // rightXOffset = rightXavg-50; rightYOffset = rightYavg-50;

    // circle(cflow, Point2f((float)15, (float)15), 10, Scalar(0,255,0), -1, 8);
    // circle(cflow, rightPupil, 3, Scalar(0,255,0), -1, 8);
    // circle(cflow, leftPupil, 3, Scalar(0,255,0), -1, 8);

    // draw eye centers
    if (DEBUG_LEVEL >= 1 && PHONE == 0) {
        imshow("main", cflow);
    }
}

int setUp(const char* cascadePath) {
    try {
        if(!face_cascade.load(cascadePath)) {
            throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
        }
    } catch (const char* msg) {
        doLog(msg);
        throw;
    }
}

TermCriteria termcrit;
Size subPixWinSize, winSize;
const int MAX_COUNT = 500;
bool addRemovePtx = false;
int flg=0;
vector<Point2f> points[2];
Mat pleft, pright;
int firstLoopProcs = 1;
int process(Mat frame, Mat gray, Mat out) {
    clock_t start;
    cv::Rect face, leftEyeRegion, rightEyeRegion;
    cv::Point leftPupil, rightPupil;
    Mat faceROI;
    Mat left, right;
    Mat flowLeft, flowRight;

    start = clock();
    if (faceDetect(gray, &face) != 0) {
        if (DEBUG_LEVEL >= 1 && PHONE == 0) {
            imshow("main", out);
        }
        return -1;
    }
    // // faceROI = gray(face);
    // if (kSmoothFaceImage) {
    //     double sigma = kSmoothFaceFactor * gray.cols*1;
    //     // todo only farne eye region
    //     GaussianBlur(gray, gray, cv::Size(0, 0), sigma);
    // }

    diffclock("- facedetect", start);

    // start = clock();
    // eyeRegions(face, &leftEyeRegion, &rightEyeRegion);
    // diffclock("- eyeRegions", start);
    // start = clock();
    // eyeCenters(faceROI, leftEyeRegion, rightEyeRegion, &leftPupil, &rightPupil);
    // diffclock("- eyeCenters", start);
    start = clock();
    getLeftRightEyeMat(gray, leftEyeRegion, rightEyeRegion,  &left, &right);
    diffclock("- getLeftRightEyeMat", start);

    if (firstLoopProcs == 0) {
        start = clock();
        calcOpticalFlowFarneback(pleft, left, flowLeft, 0.5, 3, 15, 3, 5, 1.2, 0);
        calcOpticalFlowFarneback(pright, right, flowRight, 0.5, 3, 15, 3, 5, 1.2, 0);
        diffclock("- farneback", start);

        start = clock();
        drawOptFlowMap(flowLeft, out, 10, Scalar(0, 255, 0), 0);
        drawOptFlowMap(flowRight, out, 10, Scalar(0, 255, 0), 1);
        diffclock("- drawOptFlowMap", start);

        if (DEBUG_LEVEL >= 2 && PHONE == 0) {
            imshow("left", left);
            imshow("right", right);
        }
    } else {
        firstLoopProcs = 0;
    }

    // if (flg == 1) {
    //     Size subPixWinSize(10,10), winSize(31,31);
    //     goodFeaturesToTrack(left, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
    //     // cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit); // sig abrt
    //     flg = 0;
    // } else {
    //     if(!points[0].empty()){
    //         printf("BUJU BUJU\n");
    //         Point2f point;
    //         vector<uchar> status;
    //         vector<float> err;
    //         TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
    //         Size subPixWinSize(10,10), winSize(31,31);
    //         calcOpticalFlowPyrLK(pleft, left, points[0], points[1], status, err, winSize,
    //                              3, termcrit, 0, 0.001);
    //         size_t i, k;
    //         for(i = k = 0; i < points[1].size(); i++) {
    //             if(addRemovePtx) {
    //                 if(norm(point - points[1][i]) <= 5 ) {
    //                     addRemovePtx = false;
    //                     continue;
    //                 }
    //             }
            
    //             if(!status[i]) {
    //                 continue;
    //             }
                
    //             points[1][k++] = points[1][i];
    //             circle(frame, points[1][i], 3, Scalar(0,255,0), -1, 8);
    //         }
    //         points[1].resize(k);
    //     }
    // }

    start = clock();
    showResult(out, face, faceROI, leftEyeRegion, rightEyeRegion, leftPupil, rightPupil);
    diffclock("- showResult", start);

    pleft = left.clone(); pright = right.clone(); // TODO try just with assigning
    std::swap(points[1], points[0]);
}

int main() {
    PHONE = 0; farne = 0;
    // test videos
    // char fileName[100] = "/home/developer/other/posnetki/o4_29.mp4";
    // char fileName[100] = "/home/developer/other/posnetki/o4_30.mp4";
    char fileName[100] = "/home/developer/other/posnetki/o4_31.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/crnc1.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/indian_close.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/yellow_close.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/very_dark.mp4";
    // char fileName[100] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
    VideoCapture stream1(fileName);   //0 is the id of video device.0 if you have only one camera
    // VideoCapture stream1(0);
    if (!stream1.isOpened()) {
        CV_Assert("Cam open failed");
    }

    // resolutions 320, 240; 800, 448; 640, 480
    stream1.set(CV_CAP_PROP_FRAME_WIDTH, 640); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

    char faceDetector[200] = "/home/developer/other/android_deps/OpenCV-2.4.10-android-sdk/samples/optical-flow/res/raw/lbpcascade_frontalface.xml";
    // printf("video capture %f %f %f\n", stream1.get(CV_CAP_PROP_FRAME_WIDTH), stream1.get(CV_CAP_PROP_FRAME_HEIGHT), stream1.get(CV_CAP_PROP_FPS));

    // controls
    int pause = 0, firstLoop = 1;
    clock_t loopStart, start;
    Mat frame, gray, cflow;

    setUp(faceDetector);

    if (farne == 0) {
        cv::namedWindow(face_window_name,CV_WINDOW_NORMAL); cv::moveWindow(face_window_name, 10, 100);
        cv::namedWindow("main",CV_WINDOW_NORMAL); cv::moveWindow("main", 10, 100); resizeWindow("main",1280, 960);
        // cv::namedWindow("Right Eye",CV_WINDOW_NORMAL); cv::moveWindow("Right Eye", 10, 600);
        // cv::namedWindow("Left Eye",CV_WINDOW_NORMAL); cv::moveWindow("Left Eye", 10, 800);
        // createCornerKernels(), at the end // releaseCornerKernels(); // ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2), 43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);
    }

    while (true) {
        if(!(stream1.read(frame))) {
            doLog(" --(!) No captured frame -- Break!");
            return 0;
        }
        // cv::flip(frame, frame, 1);

        if (firstLoop == 1) {
            loopStart = clock();
            firstLoop = 0;
            continue;
        }

        diffclock("\nwhole loop", loopStart);
        loopStart = clock();

        start = clock();
        getGray(frame, &gray);
        diffclock("getGray", start);

        start = clock();
        process(frame, gray, frame);
        diffclock("process", start);

        // flow control
        int c = cv::waitKey(10);
        if(c == 27) { // esc
            break;
        } else if((char)c == 'p') {
            pause = 1;
        } else if((char)c == 'f') {
            flg = 1;
            // imwrite("/tmp/frame.png",cflow);
        } else if (pause == 1) {
            while (true) {
                int c = cv::waitKey(10);
                if((char)c == 'p') {
                    pause = 0;
                    break;
                } else if((char)c == 'n') {
                    break;
                }
            }
        }
    }

    return 0;
}

///

auto startx = std::chrono::high_resolution_clock::now();

class OptFlow {
    Mat rgb, grayx, left, right, prevLeft, prevRight;
    unsigned long long int ns = 0;
    vector<Point2f> points[2];
    const int MAX_COUNT = 501;
    TermCriteria termcrit;
    Size subPixWinSize, winSize;
    
    
    bool yes = false;
    bool yes1 = false;
    Point2f point;
    
    public: OptFlow () {
        ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-startx).count();
        
        termcrit= TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
        subPixWinSize = Size(10,10);
        winSize = Size(31,31);
        
    }
    public: int setup(const char* cascadeFileName) {
        setUp(cascadeFileName);
        return 0;
    }
    public: int run(JNIEnv * jenv, Mat rgb, Mat grayo) {
        //cvtColor(rgb, grayx, COLOR_BGR2GRAY);
        //process(rgb, grayx, rgb);
        process(rgb, grayo, rgb);

        cv::swap(prevLeft, left);
        cv::swap(prevRight, right);
        
        return 0;
    }
}; // end of OptFlow class definition

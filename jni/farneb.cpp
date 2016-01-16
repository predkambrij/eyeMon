#include <stdio.h>
#include <iostream>
#include <ctime>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>


//#ifdef _DEBUG        
//#pragma comment(lib, "opencv_core247d.lib")
//#pragma comment(lib, "opencv_imgproc247d.lib")   //MAT processing
//#pragma comment(lib, "opencv_objdetect247d.lib") //HOGDescriptor
////#pragma comment(lib, "opencv_gpu247d.lib")
////#pragma comment(lib, "opencv_features2d247d.lib")
//#pragma comment(lib, "opencv_highgui247d.lib")
//#pragma comment(lib, "opencv_ml247d.lib")
////#pragma comment(lib, "opencv_stitching247d.lib");
////#pragma comment(lib, "opencv_nonfree247d.lib");
//#pragma comment(lib, "opencv_video247d.lib")
//#else
//#pragma comment(lib, "opencv_core247.lib")
//#pragma comment(lib, "opencv_imgproc247.lib")
//#pragma comment(lib, "opencv_objdetect247.lib")
////#pragma comment(lib, "opencv_gpu247.lib")
////#pragma comment(lib, "opencv_features2d247.lib")
//#pragma comment(lib, "opencv_highgui247.lib")
//#pragma comment(lib, "opencv_ml247.lib")
////#pragma comment(lib, "opencv_stitching247.lib");
////#pragma comment(lib, "opencv_nonfree247.lib");
//#pragma comment(lib, "opencv_video247d.lib")
//#endif 

using namespace cv;
using namespace std;

#define DEBUG_LEVEL 1
//#define PHONE 0

//JNIEnv* env;
int PHONE = 1;
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
int xoffset=100, yoffset=170, cols=100, rows=100;
int xoffset1=250, yoffset1=170, cols1=100, rows1=100;

void drawOptFlowMap (const Mat flow, Mat cflowmap, int step, const Scalar& color, int eye) {
    //circle(cflowmap, Point2f((float)10, (float)10), 3, Scalar(0,255,0), -1, 8);
    circle(cflowmap, Point2f((float)15, (float)15), 10, Scalar(0,255,0), -1, 8);
    int xo, yo;
    if (eye == 0) {
        xo = xoffset;
        yo = yoffset;
    } else {
        xo = xoffset1;
        yo = yoffset1;
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

int faceDetect() {
}

int preprocess(Mat frame, Mat *output, Mat *output1)
{
    int resizeFactor = 1;
    //resize(frame, *output, Size(frame.size().width/resizeFactor, frame.size().height/resizeFactor));

    frame(cv::Rect(xoffset,yoffset,cols,rows)).copyTo(*output);
    frame(cv::Rect(xoffset1,yoffset1,cols1,rows1)).copyTo(*output1);
    //frame.copyTo(output);
    if (PHONE == 0) {
        cvtColor(*output, *output, CV_BGR2GRAY);
        cvtColor(*output1, *output1, CV_BGR2GRAY);
    }
}

int process(Mat previous, Mat next, Mat previous1, Mat next1, Mat cflow)
{
    clock_t start;
    Mat flow, flow1;

    start = clock();
    calcOpticalFlowFarneback(previous, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
    calcOpticalFlowFarneback(previous1, next1, flow1, 0.5, 3, 15, 3, 5, 1.2, 0);
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
        imshow("previous", previous);
        imshow("next", next);
    }
}

int main()
{
    PHONE = 0;
    // video source
    char fileName[100] = "/home/developer/other/posnetki/o4_29.mp4";
    //char fileName[100] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
    //char fileName[100] = "mm2.avi"; //video\\mm2.avi"; //mm2.avi"; //cctv 2.mov"; //mm2.avi"; //";//_p1.avi";
    //VideoCapture stream1(fileName);   //0 is the id of video device.0 if you have only one camera   
    VideoCapture stream1(0);   //0 is the id of video device.0 if you have only one camera   

    // controls
    int pause = 0;
    int firstLoop = 1;
    clock_t loopStart;
    clock_t start;

    Mat frame;
    Mat previous, next;
    Mat previous1, next1;
    Mat cflow;
/*if (PHONE == 0) {
        cvtColor(previous, cflow, CV_GRAY2BGR);
    }*/
    
    while (true) {
        if(!(stream1.read(frame))) {
            doLog(" --(!) No captured frame -- Break!");
            return 0;
        }

        if (firstLoop == 1) {
            loopStart = clock();
            preprocess(frame, &previous, &previous1);
            firstLoop = 0;
            continue;
        }

        diffclock("\nwhole loop", loopStart);
        loopStart = clock();

        start = clock();
        preprocess(frame, &next, &next1);
        diffclock("preprocess", start);

        start = clock();
        process(previous, next, previous1, next1, frame);
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

        previous = next.clone();
        previous1 = next1.clone();
    }
    return 0;
}

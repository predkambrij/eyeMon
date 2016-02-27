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

#include <farneb.cpp>

using namespace cv;
using namespace std;

#define DEBUG_TAGx "NDK_AndroidNDK1SampleActivity"

auto startx = std::chrono::high_resolution_clock::now();
bool addRemovePtx1 = false;

void doLogx(JNIEnv * env, const char* text) {
    // malloc room for the resulting string
    char *szResult;
    szResult = (char*)malloc(sizeof(char)*500);
    
    
    auto end = std::chrono::high_resolution_clock::now();
    unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-startx).count();
    
    
    // standard sprintf
    sprintf(szResult, "res: %llu %s", ns, text);// szFormat sum CV_VERSION
    
    // get an object string
    jstring result = (env)->NewStringUTF(szResult);
    
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAGx, "NDK:OPTFLOF: [%s]", szResult);// szLogThis
    
    // cleanup
    free(szResult);
}


class OptFlow {
    Mat rgb, grayx, left, right, prevLeft, prevRight;
    unsigned long long int ns = 0;
    vector<Point2f> points[2];
    const int MAX_COUNT = 500;
    TermCriteria termcrit;
    Size subPixWinSize, winSize;
    
    
    bool yes = false;
    bool yes1 = false;
    Point2f point;
    
    public: OptFlow () {
        //for (long i=0; i<2000000000;){i++;} // TODO:
        //ns = 123;
        //auto end = std::chrono::high_resolution_clock::now();
        //ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
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
        cvtColor(rgb, grayx, COLOR_BGR2GRAY);
        process(rgb, grayx, rgb);

        cv::swap(prevLeft, left);
        cv::swap(prevRight, right);
        
        return 0;
/*
        if (yes1 == false &&
                std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-startx).count() > 3000000000) {
            goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
            char buffer [500];
            sprintf (buffer, "INIIITTT (%i)", points[1].size());
            doLogx(jenv, buffer);
            
            cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit); // sig abrt
            yes1=true;
        }
        
        
        if(!points[0].empty()){
            vector<uchar> status;
            vector<float> err;
            TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
            Size subPixWinSize(10,10), winSize(31,31);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
                                 3, termcrit, 0, 0.001);
            size_t i, k;
            for(i = k = 0; i < points[1].size(); i++) {
                if(addRemovePtx) {
                    if(norm(point - points[1][i]) <= 5 ) {
                        addRemovePtx = false;
                        continue;
                    }
                }
            
                if(!status[i]) {
                    continue;
                }
                
                points[1][k++] = points[1][i];
                circle(rgb, points[1][i], 3, Scalar(0,255,0), -1, 8);
            }
            points[1].resize(k);
        }

        
        
        
        
        circle(rgb, Point2f((float)10, (float)10), 3, Scalar(0,255,0), -1, 8);
        
        char buffer [500];
        sprintf (buffer, "OptFlow constructor: %llu (%i)", ns, points[1].size());
        doLogx(jenv, buffer);
        
        std::swap(points[1], points[0]);
        cv::swap(prevGray, gray);
        //gray.copyTo(prevGray);
        yes = true;
        return 0;*/
    }
}; // end of OptFlow class definition

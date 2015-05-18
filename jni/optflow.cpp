// fake h file
#include <jni.h>

//  JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_DetectionBasedTracker_nativeSetFaceSize
//  (JNIEnv *, jclass, jlong, jint);




#include <string.h>
#include <stdio.h>
#include <android/log.h>


#define DEBUG_TAG "NDK_AndroidNDK1SampleActivity"



#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>
#include <chrono>

using namespace cv;
using namespace std;
//using namespace std::chrono;

extern "C" {
auto start = std::chrono::high_resolution_clock::now();

//auto start = std::chrono::high_resolution_clock::now();
JNIEXPORT jstring JNICALL Java_org_opencv_samples_facedetect_FdActivity_optFlowTest(JNIEnv * env, jclass, jint value1, jint value2)
{
        //char *szFormat = "native result: %i";
        char *szResult;

        // add the two values
        jlong sum = value1+value2;

        // malloc room for the resulting string
        szResult = (char*)malloc(sizeof(char)*500);
        
        
        auto end = std::chrono::high_resolution_clock::now();
        unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        
        
        // standard sprintf
        sprintf(szResult, "r: %llu", ns);// szFormat sum

        // get an object string
        jstring result = (env)->NewStringUTF(szResult);


        __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:OPTFLOL: [%s]", szResult);// szLogThis
        
        // cleanup
        free(szResult);
        
        

        return result;
}


void doLog(JNIEnv * env, const char* text) {
    // malloc room for the resulting string
    char *szResult;
    szResult = (char*)malloc(sizeof(char)*500);
    
    
    auto end = std::chrono::high_resolution_clock::now();
    unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    
    
    // standard sprintf
    sprintf(szResult, "res: %llu %s", ns, text);// szFormat sum CV_VERSION
    
    // get an object string
    jstring result = (env)->NewStringUTF(szResult);
    
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:OPTFLOF: [%s]", szResult);// szLogThis
    
    // cleanup
    free(szResult);
}




Point2f point;
bool addRemovePt = false;
//typedef std::chrono::high_resolution_clock clock;
//auto start = clock::now();


static void onMouse( int event, int x, int y, int , void*) {
    if( event == CV_EVENT_LBUTTONDOWN )
    {
        point = Point2f((float)x, (float)y);
        addRemovePt = true;
    }
}


int run( int argc, char** argv, JNIEnv * env, jlong thiz, jlong imageGray) {
    
    
    TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
    Size subPixWinSize(10,10), winSize(31,31);

    const int MAX_COUNT = 500;
    bool needToInit = false;
    bool nightMode = false;


    Mat gray, prevGray, image;
    vector<Point2f> points[2];
    bool stop_it = false;
    
    
    {
        
        if (stop_it == false) {
            
            
            auto end = std::chrono::high_resolution_clock::now();
            unsigned long long int ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
            if (ns > 4000000000) {
                stop_it = true;
                std::cout << ns << "ns\n";
                needToInit = true;
            }
            
        }
        
        
        Mat frame;
        
        if( frame.empty() )
            //doLog(env, "fame empty, return");
            return 1;

        frame.copyTo(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);

        if( nightMode )
            image = Scalar::all(0);

        if( needToInit )
        {
            // automatic initialization
            goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
            cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
            addRemovePt = false;
            
            
            
            
        }
        else if( !points[0].empty() )
        {
            vector<uchar> status;
            vector<float> err;
            if(prevGray.empty())
                gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
                                 3, termcrit, 0, 0.001);
            size_t i, k;
            for( i = k = 0; i < points[1].size(); i++ )
            {
                if( addRemovePt )
                {
                    if( norm(point - points[1][i]) <= 5 )
                    {
                        addRemovePt = false;
                        continue;
                    }
                }

                if( !status[i] )
                    continue;

                points[1][k++] = points[1][i];
                circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
            }
            points[1].resize(k);
        }

        if( addRemovePt && points[1].size() < (size_t)MAX_COUNT )
        {
            vector<Point2f> tmp;
            tmp.push_back(point);
            cornerSubPix( gray, tmp, winSize, cvSize(-1,-1), termcrit);
            points[1].push_back(tmp[0]);
            addRemovePt = false;
        }

        needToInit = false;
        
        //imshow("LK Demo", image);


        std::swap(points[1], points[0]);
        cv::swap(prevGray, gray);
    }

    return 0;
}
JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_FdActivity_optFlow(JNIEnv * env, jclass, jlong thiz, jlong imageGray) {
    int argc = 0;
    char** argv;
    
    //run(env, thiz, imageGray);
    doLog(env, "KONEC");
    
}
JNIEXPORT jlong JNICALL Java_org_opencv_samples_facedetect_OptFlow_optFlowCreateObject
                                                                            (JNIEnv * jenv, jclass, jstring jFileName, jint faceSize) {
    jlong result = 0;
    return result;
}

JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_OptFlow_optFlowDestroyObject(JNIEnv * jenv, jclass, jlong thiz) {
}
class OptFlow {
    Mat rgb, gray, prevGray;
    unsigned long long int ns = 0;
    vector<Point2f> points[2];
    const int MAX_COUNT = 500;
    TermCriteria termcrit;
    Size subPixWinSize, winSize;
    
    
    bool yes = false;
    bool yes1 = false;
    Point2f point;
    
    public:
        OptFlow () {
            //for (long i=0; i<2000000000;){i++;} // TODO:
            //ns = 123;
            //auto end = std::chrono::high_resolution_clock::now();
            //ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
            ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-start).count();
            
            termcrit= TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
            subPixWinSize = Size(10,10);
            winSize = Size(31,31);
            
        }
        int run(JNIEnv * jenv, Mat rgb, Mat grayo) {
            cvtColor(rgb, gray, COLOR_BGR2GRAY);
            if(prevGray.empty()) {
                gray.copyTo(prevGray);
                /*
                point = Point2f((float)10, (float)30);
                vector<Point2f> tmp;
                tmp.push_back(point);
                cornerSubPix( gray, tmp, winSize, cvSize(-1,-1), termcrit);
                points[1].push_back(tmp[0]);
                */
                
            }
            if (yes1 == false &&
                    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-start).count() > 1000000000) {
                goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
                char buffer [500];
                sprintf (buffer, "INIIITTT (%i)", points[1].size());
                doLog(jenv, buffer);
                
                cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit); // sig abrt
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
                    if(addRemovePt) {
                        if(norm(point - points[1][i]) <= 5 ) {
                            addRemovePt = false;
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
            doLog(jenv, buffer);
            
            std::swap(points[1], points[0]);
            cv::swap(prevGray, gray);
            //gray.copyTo(prevGray);
            yes = true;
            return 0;
        }
};
OptFlow optf;

JNIEXPORT void JNICALL Java_org_opencv_samples_facedetect_OptFlow_optFlowDetect
                                                                (JNIEnv * jenv, jclass, jlong imageRGB, jlong imageGray) {
    Mat rgb = *((Mat*)imageRGB);
    Mat gray = *((Mat*)imageGray);
        
    optf.run(jenv, rgb, gray);
    
    doLog(jenv, "end of detectX");
}


}


/*

*/

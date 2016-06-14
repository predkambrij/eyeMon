
#ifdef IS_PHONE
#include <jni.h>
#include <android/log.h>
#endif

#include <string.h>
#include <stdio.h>

#include <iostream>
#include <ctype.h>
#include <chrono>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>

#include <eyelike/main.cpp>

#include <common.hpp>
#include <optflow.hpp>

void drawOptFlowMap (const cv::Mat flow, cv::Mat cflowmap, int step, const cv::Scalar& color, int eye) {
    //cv::circle(cflowmap, cv::Point2f((float)10, (float)10), 3, cv::Scalar(0,255,0), -1, 8);
    cv::circle(cflowmap, cv::Point2f((float)15, (float)15), 10, cv::Scalar(0,255,0), -1, 8);
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
            
            const cv::Point2f& fxy = flow.at< cv::Point2f>(y, x);
            int px = x+xo, py = y+yo;
            cv::line(cflowmap, cv::Point(px,py), cv::Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), color);
            //circle(cflowmap, Point(cvRound(x+fxy.x), cvRound(y+fxy.y)), 1, color, -1);
            cv::circle(cflowmap, cv::Point(cvRound(px+fxy.x), cvRound(py+fxy.y)), 1, color, -1, 8);

            //circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
            printf("%.0f ", fxy.y);
        }
        printf("\n");
    }
    printf("\n\n\n");
}

void getLeftRightEyeMat(cv::Mat gray, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Mat *left, cv::Mat *right) {
    // printf("eye_region_width %d, eye_region_height %d\n", eye_region_width, eye_region_height);
    // printf("leftEyeRegion %d, leftEyeRegion %d\n", leftEyeRegion.x, leftEyeRegion.y);
    // printf("rightEyeRegion %d, rightEyeRegion %d\n", rightEyeRegion.x, rightEyeRegion.y);
    // gray(cv::Rect(leftEyeRegion.x, leftEyeRegion.y, 60, 70)).copyTo(*left);
    // gray(cv::Rect(rightEyeRegion.x, rightEyeRegion.y, 60, 70)).copyTo(*right);
    gray(cv::Rect(leftXOffset, leftYOffset, leftCols, leftRows)).copyTo(*left);
    gray(cv::Rect(rightXOffset, rightYOffset, rightCols, rightRows)).copyTo(*right);
}

int faceDetect(cv::Mat gray, cv::Rect *face) {
    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
    if (faces.size() != 1) {
        return -1;
    }
    *face = faces[0];
    return 0;
}
void eyeRegions(cv::Rect face, cv::Rect *leftEyeRegion, cv::Rect *rightEyeRegion) {
    eye_region_width = face.width * (kEyePercentWidth/100.0);
    eye_region_height = face.width * (kEyePercentHeight/100.0);
    int eye_region_top = face.height * (kEyePercentTop/100.0);
    (*leftEyeRegion) = cv::Rect(face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
    (*rightEyeRegion) = cv::Rect(face.width - eye_region_width - face.width*(kEyePercentSide/100.0), eye_region_top, eye_region_width, eye_region_height);
}
void eyeCenters(cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point *leftPupil, cv::Point *rightPupil) {
    //*leftPupil  = findEyeCenter(faceROI, leftEyeRegion, "Left Eye");
    //*rightPupil = findEyeCenter(faceROI, rightEyeRegion, "Right Eye");
}

void showResult(cv::Mat cflow, cv::Rect face, cv::Mat faceROI, cv::Rect leftEyeRegion, cv::Rect rightEyeRegion, cv::Point leftPupil, cv::Point rightPupil) {
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
    if (debug_show_img_main == true && PHONE == 0) {
        imshow("main", cflow);
    }
}

void OptFlow::process(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    clock_t start;
    cv::Point leftPupil, rightPupil;
//    cv::Rect face, leftEyeRegion, rightEyeRegion;
    cv::Mat faceROI;
    cv::Mat left, right;
    cv::Mat flowLeft, flowRight;
/*
    start = clock();
    if (faceDetect(gray, &face) != 0) {
        if (debug_show_img_main == true && PHONE == 0) {
            imshow("main", out);
        }
        return;
    }
*/
    // // faceROI = gray(face);
    // if (kSmoothFaceImage) {
    //     double sigma = kSmoothFaceFactor * gray.cols*1;
    //     // todo only farne eye region
    //     GaussianBlur(gray, gray, cv::Size(0, 0), sigma);
    // }
/*
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
        cv::calcOpticalFlowFarneback(pleft, left, flowLeft, 0.5, 3, 15, 3, 5, 1.2, 0);
        cv::calcOpticalFlowFarneback(pright, right, flowRight, 0.5, 3, 15, 3, 5, 1.2, 0);
        diffclock("- farneback", start);

        start = clock();
        drawOptFlowMap(flowLeft, out, 10, cv::Scalar(0, 255, 0), 0);
        drawOptFlowMap(flowRight, out, 10, cv::Scalar(0, 255, 0), 1);
        diffclock("- drawOptFlowMap", start);

        if (debug_show_img_optfl_eyes == true && PHONE == 0) {
            imshow("left", left);
            imshow("right", right);
        }
    } else {
        firstLoopProcs = 0;
    }
*/
    if ((frameNum % 90) == 0) {
        // initialize
        flg=1;
    }
    if (flg == 1) {
        if (faceDetect(gray, &face) != 0) {
            if (debug_show_img_main == true && PHONE == 0) {
                imshow("main", out);
            }
            return;
        }

        int rowsO = (face).height/4.3;
        int colsO = (face).width/5.5;
        int rows2 = (face).height/4.3;
        int cols2 = (face).width/4.5;
        leftE = cv::Rect(colsO, rowsO, cols2, rows2);
        rightE = cv::Rect((face).width-colsO-rows2, rowsO, cols2, rows2);
    }
    //printf("face %d %d\n", face.width, face.height);
    faceROI = gray(face);
    left  = faceROI(leftE);
    right = faceROI(rightE);
    imshowWrapper("face", faceROI, debug_show_img_face);
    imshowWrapper("leftSR", left, debug_show_img_templ_eyes_cor);
    imshowWrapper("rightSR", right, debug_show_img_templ_eyes_cor);

    if (flg == 1) {
        start = clock();

        printf("BUJU init %d\n", frameNum);
        cv::Size subPixWinSize(10,10), winSize(31,31);
        cv::goodFeaturesToTrack(left, points[1], MAX_COUNT, 0.01, 10, cv::Mat(), 3, false, 0.04);
        printf("BUJU init res %d %lu\n", frameNum, points[1].size());
        // cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit); // sig abrt
        flg = 0;
     } else {
        if(!points[0].empty()){
            cv::Point2f point;
            std::vector<uchar> status;
            std::vector<float> err;
            cv::TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
            cv::Size subPixWinSize(10,10), winSize(31,31);
            
            cv::calcOpticalFlowPyrLK(pleft, left, points[0], points[1], status, err
                                , winSize, 3, termcrit, 0, 0.001
                                );
            
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
                //cv::circle(out, points[1][i], 3, cv::Scalar(0,255,0), -1, 8);
                cv::circle(out, cv::Point2f((float)points[1][i].x+face.x+leftE.x, (float)points[1][i].y+face.y+leftE.y), 3, cv::Scalar(0,255,0), -1, 8);
            }
            points[1].resize(k);
        }
    }
/*
*/
    start = clock();
    showResult(out, face, faceROI, leftEyeRegion, rightEyeRegion, leftPupil, rightPupil);
    diffclock("- showResult", start);

    pleft = left.clone(); pright = right.clone();
    std::swap(points[1], points[0]);
}

OptFlow::OptFlow () {
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-startx).count();
    
    termcrit= cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 20, 0.03);
    subPixWinSize = cv::Size(10,10);
    winSize = cv::Size(31,31);
};

int OptFlow::setup(const char* cascadeFileName) {
    try {
        if(!face_cascade.load(cascadeFileName)) {
            throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
        }
    } catch (const char* msg) {
        doLog(true, msg);
        throw;
    }
    return 0;
};

#ifdef IS_PHONE
int OptFlow::setJni(JNIEnv* jenv) {
};
#endif

int OptFlow::run(cv::Mat gray, cv::Mat out, double timestamp, unsigned int frameNum) {
    //cvtColor(rgb, grayx, COLOR_BGR2GRAY);
    //process(rgb, grayx, rgb);
    process(gray, out, timestamp, frameNum);

    //cv::swap(prevLeft, left);
    //cv::swap(prevRight, right);
    return 0;
};

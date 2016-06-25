#ifndef BLINKMEASUREF_H
#define BLINKMEASUREF_H

#include <opencv2/core/core.hpp>


class BlinkMeasureF {
    public:
        // state machine
        static int lCurState;
        static double lLastVal;
        static int lZeroCrossPosToNegF;
        static double lZeroCrossPosToNegT;
        static int rCurState;
        static double rLastVal;
        static int rZeroCrossPosToNegF;
        static double rZeroCrossPosToNegT;

        unsigned int frameNum;
        double timestamp;
        cv::Point2d lDiffP;
        cv::Point2d rDiffP;
        bool canProceedL, canProceedR;
        // converting to chuncks
        static bool lAdding;
        static double lFirstBlinkT;
        static double lLastNonBlinkT;
        static unsigned int lFirstBlinkF;
        static unsigned int lLastNonBlinkF;
        static bool rAdding;
        static double rFirstBlinkT;
        static double rLastNonBlinkT;
        static unsigned int rFirstBlinkF;
        static unsigned int rLastNonBlinkF;
        static double maxNonBlinkT;

        BlinkMeasureF(unsigned int frameNum, double timestamp, cv::Point2d lDiffP, cv::Point2d rDiffP, bool canProceedL, bool canProceedR);
        static void measureBlinks();
        static void stateMachine(unsigned int frameNum, double timestamp, double leftY, double leftLowSD, double leftHighSD, double rightY, double rightLowSD, double rightHighSD);
        static void measureBlinksAVG(double *lavg, double *ravg);
        static void measureBlinksSD(double lavg, double ravg, double *lSD, double *rSD, double *plsd1, double *prsd1, double *plsd2, double *prsd2, double *mlsd1, double *mrsd1, double *mlsd2, double *mrsd2);
        static void makeChunk(bool isLeft, double timestamp, bool isBlink, unsigned int frameNum);
        static void makeNotification(bool isLeft);
};

class BlinkF {
    public:
        unsigned int frameStart;
        unsigned int frameEnd;
        double timestampStart;
        double timestampEnd;
        int eventType;
        //double corAvg;
        BlinkF(unsigned int frameStart, unsigned int frameEnd, double timestampStart, double timestampEnd, int eventType);
};

#endif


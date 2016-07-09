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
        bool canProceedL, canProceedR, canUpdateL, canUpdateR;
        // converting to chuncks
        static bool n1UnderThreshold;
        static bool lAdding;
        static unsigned int lastAddedToStateMachine;
        static bool delayStateMachine;
        static double startTS;
        static double prevTS;
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

        BlinkMeasureF(unsigned int frameNum, double timestamp, cv::Point2d lDiffP, cv::Point2d rDiffP, bool canProceedL, bool canProceedR, bool canUpdateL, bool canUpdateR);
        BlinkMeasureF();
        static void rewriteElementsToStateQueue(unsigned int frameNum, double timestamp);
        static bool measureBlinks(BlinkMeasureF bm);
        static void processBm(BlinkMeasureF bm);
        static void processStateMachineQueue();
        static bool joinBlinks();
        static void processBm(BlinkMeasureF bm,
            double lavg, double ravg, double lSD, double rSD,
            double mlsdt, double plsdt, double mrsdt, double prsdt,
            double plsd1, double prsd1, double mlsd1, double mrsd1,
            double plsd2, double prsd2, double mlsd2, double mrsd2);
        static bool checkN1Notifs(double curTimestamp);
        static void stateMachine(unsigned int frameNum, double timestamp, double leftY, double leftLowSD, double leftHighSD, double rightY, double rightLowSD, double rightHighSD);
        static void measureBlinksAVG(double *lavg, double *ravg);
        static void measureBlinksSD(double *lSD, double *rSD, double *plsd1, double *prsd1, double *plsd2, double *prsd2, double *plsdt, double *prsdt, double *mlsd1, double *mrsd1, double *mlsd2, double *mrsd2, double *mlsdt, double *mrsdt);
        static void measureSD(double* mlsdt, double* plsdt, double* mrsdt, double* prsdt,
            double* lavg, double* ravg, double* lSD, double* rSD,
            double* plsd1, double* prsd1, double* mlsd1, double* mrsd1,
            double* plsd2, double* prsd2, double* mlsd2, double* mrsd2);
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


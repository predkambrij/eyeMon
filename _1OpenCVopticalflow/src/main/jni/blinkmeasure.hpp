
#ifndef BLINKMEASURE_H
#define BLINKMEASURE_H


class BlinkMeasure {
    public:
        unsigned int frameNum;
        double timestamp;
        double lcor;
        double rcor;
        bool canProceedL, canProceedR, canUpdateL, canUpdateR;
        // converting to chuncks
        static bool n1UnderThreshold;
        static double startTS;
        static double prevTS;
        static bool lAdding;
        static unsigned int lastAddedToStateMachine;
        static bool delayStateMachine;
        static bool isFirst;
        static double prevLcor;
        static double prevRcor;
        static double lFirstBlinkT;
        static double lLastNonBlinkT;
        static unsigned int lFirstBlinkF;
        static unsigned int lLastNonBlinkF;
        static bool rAdding;
        static double rFirstBlinkT;
        static double rLastNonBlinkT;
        static unsigned int rFirstBlinkF;
        static unsigned int rLastNonBlinkF;
        static unsigned int lLastAddedFN;
        static unsigned int rLastAddedFN;
        static double maxNonBlinkT;

        BlinkMeasure();
        BlinkMeasure(unsigned int frameNum, double timestamp, double lcor, double rcor);
        static void rewriteElementsToStateQueue(unsigned int frameNum, double timestamp);
        static bool measureBlinks(BlinkMeasure bm);
        static void processStateMachineQueue();
        static void processBm(BlinkMeasure bm, double lavg, double ravg, double lSD, double rSD,
            double plsdf, double mlsdf, double prsdf, double mrsdf, double lsdt, double rsdt);
        static bool checkN1Notifs(double curTimestamp);
        static bool joinBlinks();
        static void measureBlinksAVG(double *lavg, double *ravg);
        static void measureBlinksSD(double *lSD, double *rSD, double *lsdt, double *rsdt, double *plsdf, double *prsdf, double *mlsdf, double *mrsdf);
        static void makeChunk(bool isLeft, double timestamp, bool isBlink, unsigned int frameNum);
        static void makeNotification(bool isLeft);
};

class Blink {
    public:
        unsigned int frameStart;
        unsigned int frameEnd;
        double timestampStart;
        double timestampEnd;
        int eventType;
        //double corAvg;
        Blink(unsigned int frameStart, unsigned int frameEnd, double timestampStart, double timestampEnd, int eventType);
};

#endif

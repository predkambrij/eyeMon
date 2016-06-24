
#ifndef BLINKMEASURE_H
#define BLINKMEASURE_H


class BlinkMeasure {
    public:
        unsigned int frameNum;
        double timestamp;
        double lcor;
        double rcor;
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

        BlinkMeasure(unsigned int frameNum, double timestamp, double lcor, double rcor);
        static void measureBlinks();
        static void measureBlinksAVG(int shortBmSize, double *lavg, double *ravg);
        static void measureBlinksSD(int shortBmSize, double lavg, double ravg, double *lSD, double *rSD, double *lsd1, double *rsd1, double *lsd2, double *rsd2);
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

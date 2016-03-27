
#ifndef BLINKMEASURE_H
#define BLINKMEASURE_H

void cMeasureBlinks();

class BlinkMeasure {
    public:
        double timestamp;
        double lcor;
        double rcor;
        // converting to chuncks
        static bool lAdding;
        static double lFirstBlinkT;
        static double lLastNonBlinkT;
        static bool rAdding;
        static double rFirstBlinkT;
        static double rLastNonBlinkT;
        static double maxNonBlinkT;

        BlinkMeasure(double timestamp, double lcor, double rcor);
        static void measureBlinks();
        static void measureBlinksAVG(int shortBmSize, double *lavg, double *ravg);
        static void measureBlinksSD(int shortBmSize, double lavg, double ravg, double *lSD, double *rSD, double *lsd1, double *rsd1, double *lsd2, double *rsd2);
        static void makeChunk(bool isLeft, double timestamp, bool isBlink);
};

class Blink {
    public:
        double timestampStart;
        double timestampEnd;
        //double corAvg;
        Blink(double timestampStart, double timestampEnd);
};

#endif

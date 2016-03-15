
#ifndef BLINKMEASURE_H
#define BLINKMEASURE_H

void cMeasureBlinks();

class BlinkMeasure {
    public:
        double timestamp;
        double lcor;
        double rcor;
        BlinkMeasure(double timestamp, double lcor, double rcor);
        static void measureBlinks();
        static void measureBlinksAVG(int shortBmSize, double *lavg, double *ravg);
        static void measureBlinksSD(int shortBmSize, double lavg, double ravg, double *lSD, double *rSD, double *lsd1, double *rsd1, double *lsd2, double *rsd2);
};

#endif

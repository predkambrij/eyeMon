
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
};

#endif

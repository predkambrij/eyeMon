#include <list>
#include <stdio.h>

#include <common.hpp>
#include <blinkmeasure.hpp>

BlinkMeasure::BlinkMeasure(double timestamp, double lcor, double rcor) {
    this->timestamp = timestamp;
    this->lcor = lcor;
    this->rcor = rcor;
};

void BlinkMeasure::measureBlinks() {
    long unsigned int blinkMeasureSize = blinkMeasure.size();
    if (blinkMeasureSize == 0) {
        printf("blinkMeasureSize is zero\n");
        return;
    }

    BlinkMeasure bm = blinkMeasure.front();
    blinkMeasure.pop_front();

    blinkMeasureShort.push_back(bm);
    while (true) {
        BlinkMeasure oldestBm = blinkMeasureShort.front();
        printf("tp1 %lf tp2 %lf\n", oldestBm.timestamp, bm.timestamp);
        if (oldestBm.timestamp > (bm.timestamp - 3000)) {
            break;
        } else {
            blinkMeasureShort.pop_front();
        }
    }

    int shortBmSize = blinkMeasureShort.size();
    if (shortBmSize < 60) {
        printf("shortBmSize is less than X %d\n", shortBmSize);
        return;
    }

    double lavg = 0;
    double ravg = 0;
    std::list<BlinkMeasure>::iterator iter = blinkMeasureShort.begin();
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        lavg += bm.lcor;
        ravg += bm.rcor;
        iter++;
    }
    lavg = lavg/shortBmSize;
    ravg = ravg/shortBmSize;
    printf("sbm2 lavg:%.8lf ravg:%.8lf\n", lavg, ravg);
}

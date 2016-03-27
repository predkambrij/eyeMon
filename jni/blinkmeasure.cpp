#include <list>
#include <stdio.h>

#include <math.h>
#include <common.hpp>
#include <blinkmeasure.hpp>


BlinkMeasure::BlinkMeasure(double timestamp, double lcor, double rcor) {
    this->timestamp = timestamp;
    this->lcor = lcor;
    this->rcor = rcor;
};

void BlinkMeasure::measureBlinksAVG(int shortBmSize, double *lavg, double *ravg) {
    std::list<BlinkMeasure>::iterator iter = blinkMeasureShort.begin();
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        *lavg += bm.lcor;
        *ravg += bm.rcor;
        iter++;
    }
    *lavg = *lavg/shortBmSize;
    *ravg = *ravg/shortBmSize;
    doLog(debug_blinks_d1, "BM: lavg:%.8lf ravg:%.8lf\n", *lavg, *ravg);
};

void BlinkMeasure::measureBlinksSD(int shortBmSize, double lavg, double ravg, double *lSD, double *rSD, double *lsd1, double *rsd1, double *lsd2, double *rsd2) {
    std::list<BlinkMeasure>::iterator iter = blinkMeasureShort.begin();
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        *lSD = *lSD+pow(lavg-bm.lcor, 2);
        *rSD = *rSD+pow(ravg-bm.lcor, 2);
        iter++;
    }
    *lSD = pow(*lSD/shortBmSize, 0.5);
    *rSD = pow(*rSD/shortBmSize, 0.5);
    *lsd1 = lavg-(1*(*lSD));
    *rsd1 = ravg-(1*(*rSD));
    *lsd2 = lavg-(2*(*lSD));
    *rsd2 = ravg-(2*(*rSD));
    doLog(debug_blinks_d1, "BM: lSD %lf, rSD %lf, lsd1 %lf, rsd1 %lf, lsd2 %lf, rsd2 %lf\n", *lSD, *rSD, *lsd1, *rsd1, *lsd2, *rsd2);
};

void BlinkMeasure::measureBlinks() {
    long unsigned int blinkMeasureSize = blinkMeasure.size();
    if (blinkMeasureSize == 0) {
        doLog(debug_blinks_d1, "BM: blinkMeasureSize is zero\n");
        return;
    }

    BlinkMeasure bm = blinkMeasure.front();
    blinkMeasure.pop_front();

    blinkMeasureShort.push_back(bm);
    while (true) {
        BlinkMeasure oldestBm = blinkMeasureShort.front();
        if (oldestBm.timestamp > (bm.timestamp - 5000)) {
            break;
        } else {
            blinkMeasureShort.pop_front();
        }
    }

    int shortBmSize = blinkMeasureShort.size();
    if (shortBmSize < 30) {
        doLog(debug_blinks_d1, "BM: shortBmSize is less than X %d\n", shortBmSize);
        return;
    }

    double lavg = 0;
    double ravg = 0;
    BlinkMeasure::measureBlinksAVG(shortBmSize, &lavg, &ravg);
    double lSD = 0;
    double rSD = 0;
    double lsd1 = 0;
    double rsd1 = 0;
    double lsd2 = 0;
    double rsd2 = 0;
    BlinkMeasure::measureBlinksSD(shortBmSize, lavg, ravg, &lSD, &rSD, &lsd1, &rsd1, &lsd2, &rsd2);

    doLog(debug_blinks_d2, "BM: last T %.2lf L %lf R %lf\n", bm.timestamp, bm.lcor, bm.rcor);

    if (bm.lcor < lsd1) {
        doLog(debug_blinks_d3, "BM: BLINK T %.2lf L %lf SD1 %lf SD2 %lf\n", bm.timestamp, bm.lcor, lsd1, lsd2);
        // check whether we can create a new blink (chunk)
        BlinkMeasure::makeChunk(true, (double)bm.timestamp, true);
    } else {
        BlinkMeasure::makeChunk(true, (double)bm.timestamp, false);
    }
    if (bm.rcor < rsd1) {
        doLog(debug_blinks_d3, "BM: BLINK T %.2lf R %lf SD1 %lf SD2 %lf\n", bm.timestamp, bm.rcor, rsd1, rsd2);
        // check whether we can create a new blink (chunk)
        BlinkMeasure::makeChunk(false, (double)bm.timestamp, true);
    } else {
        BlinkMeasure::makeChunk(false, (double)bm.timestamp, false);
    }
}

double BlinkMeasure::maxNonBlinkT = 0.03;
bool BlinkMeasure::lAdding = false;
bool BlinkMeasure::rAdding = false;
double BlinkMeasure::lLastNonBlinkT = -1;
double BlinkMeasure::rLastNonBlinkT = -1;

double BlinkMeasure::lFirstBlinkT = 0;
double BlinkMeasure::rFirstBlinkT = 0;

void BlinkMeasure::makeChunk(bool isLeft, double timestamp, bool isBlink) {
    if (isLeft == true) {
        if (isBlink == true) {
            if (BlinkMeasure::lAdding == false) {
                BlinkMeasure::lAdding = true;
                BlinkMeasure::lFirstBlinkT = timestamp;
            }
            BlinkMeasure::lLastNonBlinkT = -1;
        } else {
            if (BlinkMeasure::lAdding == true) {
                if (BlinkMeasure::lLastNonBlinkT == -1) {
                    BlinkMeasure::lLastNonBlinkT = timestamp;
                } else {
                    if (BlinkMeasure::lLastNonBlinkT < (timestamp-BlinkMeasure::maxNonBlinkT)) {
                        Blink b(BlinkMeasure::lFirstBlinkT, BlinkMeasure::lLastNonBlinkT);
                        lBlinkChunks.push_back(b);
                        doLog(debug_blinks_d3, "BM: adding lBlinkChunks start %.2lf end %lf duration %lf\n", BlinkMeasure::lFirstBlinkT, BlinkMeasure::lLastNonBlinkT, BlinkMeasure::lLastNonBlinkT-BlinkMeasure::lFirstBlinkT);
                        BlinkMeasure::lAdding = false;
                    }
                }
            }
        }
    } else {
        // right
        if (isBlink == true) {
            if (BlinkMeasure::rAdding == false) {
                BlinkMeasure::rAdding = true;
                BlinkMeasure::rFirstBlinkT = timestamp;
            }
            BlinkMeasure::rLastNonBlinkT = -1;
        } else {
            if (BlinkMeasure::rAdding == true) {
                if (BlinkMeasure::rLastNonBlinkT == -1) {
                    BlinkMeasure::rLastNonBlinkT = timestamp;
                } else {
                    if (BlinkMeasure::rLastNonBlinkT < (timestamp-BlinkMeasure::maxNonBlinkT)) {
                        Blink b(BlinkMeasure::rFirstBlinkT, BlinkMeasure::rLastNonBlinkT);
                        rBlinkChunks.push_back(b);
                        doLog(debug_blinks_d3, "BM: adding rBlinkChunks start %.2lf end %lf duration %lf\n", BlinkMeasure::rFirstBlinkT, BlinkMeasure::rLastNonBlinkT, BlinkMeasure::rLastNonBlinkT-BlinkMeasure::rFirstBlinkT);
                        BlinkMeasure::rAdding = false;
                    }
                }
            }
        }
    }
};

Blink::Blink(double timestampStart, double timestampEnd) {
    this->timestampStart = timestampStart;
    this->timestampEnd   = timestampEnd;
}

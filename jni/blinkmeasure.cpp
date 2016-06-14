#include <list>
#include <stdio.h>

#include <math.h>
#include <common.hpp>
#include <blinkmeasure.hpp>


BlinkMeasure::BlinkMeasure(unsigned int frameNum, double timestamp, double lcor, double rcor) {
    this->frameNum  = frameNum;
    this->timestamp = timestamp;
    this->lcor      = lcor;
    this->rcor      = rcor;
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
};

void BlinkMeasure::measureBlinks() {
    if (blinkMeasure.size() == 0) {
        doLog(debug_blinks_d1, "debug_blinks_d1: blinkMeasureSize is zero\n");
        return;
    }

    BlinkMeasure bm = blinkMeasure.front();
    blinkMeasure.pop_front();

    blinkMeasureShort.push_back(bm);
    int timeWindow = 5;
    while (true) {
        BlinkMeasure oldestBm = blinkMeasureShort.front();
        if (oldestBm.timestamp > (bm.timestamp - (timeWindow*1000))) {
            break;
        } else {
            blinkMeasureShort.pop_front();
        }
    }

    int shortBmSize = blinkMeasureShort.size();
    if (maxFramesShortList == 0) {
        if (shortBmSize < 30) {
            return;
        }
        BlinkMeasure first = blinkMeasureShort.front();
        BlinkMeasure last = blinkMeasureShort.back();
        double tsDiff = last.timestamp-first.timestamp;
        double fps = shortBmSize/(tsDiff/1000);
        maxFramesShortList = fps*timeWindow*0.80;
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d fps of the first 30 frames %lf current maxFramesShortList %d\n",
            bm.frameNum, fps, maxFramesShortList);
    } else {
        if (shortBmSize > maxFramesShortList) {
            maxFramesShortList = shortBmSize;
            doLog(debug_blinks_d1, "debug_blinks_d1: updated maxFramesShortList %d\n", maxFramesShortList);
        }
    }
    if (shortBmSize < (maxFramesShortList/2)) {
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d shortBmSize is less than max/2 %d T %lf\n", bm.frameNum, shortBmSize, bm.timestamp);
        return;
    } else {
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d shortBmSize is big enough %d\n", bm.frameNum, shortBmSize);
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
    doLog(debug_blinks_d1, "debug_blinks_d1: lastF %d T %.2lf La %lf %.8lf Ra %lf %.8lf lSD12 %lf %lf %lf rSD12 %lf %lf %lf\n",
        bm.frameNum, bm.timestamp, bm.lcor, lavg, bm.rcor, ravg, lSD, lsd1, lsd2, rSD, rsd1, rsd2);

    if (bm.lcor < lsd2) {
        doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf L %lf SD1 %lf SD2 %lf\n", bm.frameNum, bm.timestamp, bm.lcor, lsd1, lsd2);
        // check whether we can create a new blink (chunk)
        BlinkMeasure::makeChunk(true, (double)bm.timestamp, true, bm.frameNum);
    } else {
        BlinkMeasure::makeChunk(true, (double)bm.timestamp, false, bm.frameNum);
    }
    if (bm.rcor < rsd2) {
        doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf R %lf SD1 %lf SD2 %lf\n", bm.frameNum, bm.timestamp, bm.rcor, rsd1, rsd2);
        // check whether we can create a new blink (chunk)
        BlinkMeasure::makeChunk(false, (double)bm.timestamp, true, bm.frameNum);
    } else {
        BlinkMeasure::makeChunk(false, (double)bm.timestamp, false, bm.frameNum);
    }
}

double BlinkMeasure::maxNonBlinkT = 0.03;
bool BlinkMeasure::lAdding = false;
bool BlinkMeasure::rAdding = false;
double BlinkMeasure::lLastNonBlinkT = -1;
double BlinkMeasure::rLastNonBlinkT = -1;
unsigned int BlinkMeasure::lLastNonBlinkF = 0;
unsigned int BlinkMeasure::rLastNonBlinkF = 0;

double BlinkMeasure::lFirstBlinkT = 0;
double BlinkMeasure::rFirstBlinkT = 0;
unsigned int BlinkMeasure::lFirstBlinkF = 0;
unsigned int BlinkMeasure::rFirstBlinkF = 0;

void BlinkMeasure::makeChunk(bool isLeft, double timestamp, bool isBlink, unsigned int frameNum) {
    if (isLeft == true) {
        if (isBlink == true) {
            if (BlinkMeasure::lAdding == false) {
                BlinkMeasure::lAdding = true;
                BlinkMeasure::lFirstBlinkT = timestamp;
                BlinkMeasure::lFirstBlinkF = frameNum;
            }
            BlinkMeasure::lLastNonBlinkT = -1;
        } else {
            if (BlinkMeasure::lAdding == true) {
                if (BlinkMeasure::lLastNonBlinkT == -1) {
                    BlinkMeasure::lLastNonBlinkT = timestamp;
                    BlinkMeasure::lLastNonBlinkF = frameNum;
                } else {
                    if (BlinkMeasure::lLastNonBlinkT < (timestamp-BlinkMeasure::maxNonBlinkT)) {
                        Blink b(BlinkMeasure::lFirstBlinkF, BlinkMeasure::lLastNonBlinkF,
                            BlinkMeasure::lFirstBlinkT, BlinkMeasure::lLastNonBlinkT, 0);
                        lBlinkChunks.push_back(b);
                        doLog(debug_blinks_d4, "debug_blinks_d4: adding_lBlinkChunks fs %d fe %d start %.2lf end %lf duration %lf\n",
                            BlinkMeasure::lFirstBlinkF, BlinkMeasure::lLastNonBlinkF, BlinkMeasure::lFirstBlinkT, BlinkMeasure::lLastNonBlinkT, BlinkMeasure::lLastNonBlinkT-BlinkMeasure::lFirstBlinkT);
                        BlinkMeasure::makeNotification(true);
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
                BlinkMeasure::rFirstBlinkF = frameNum;
            }
            BlinkMeasure::rLastNonBlinkT = -1;
        } else {
            if (BlinkMeasure::rAdding == true) {
                if (BlinkMeasure::rLastNonBlinkT == -1) {
                    BlinkMeasure::rLastNonBlinkT = timestamp;
                    BlinkMeasure::rLastNonBlinkF = frameNum;
                } else {
                    if (BlinkMeasure::rLastNonBlinkT < (timestamp-BlinkMeasure::maxNonBlinkT)) {
                        Blink b(BlinkMeasure::rFirstBlinkF, BlinkMeasure::rLastNonBlinkF,
                            BlinkMeasure::rFirstBlinkT, BlinkMeasure::rLastNonBlinkT, 0);
                        rBlinkChunks.push_back(b);
                        doLog(debug_blinks_d4, "debug_blinks_d4: adding_rBlinkChunks fs %d fe %d start %.2lf end %lf duration %lf\n",
                            BlinkMeasure::rFirstBlinkF, BlinkMeasure::rLastNonBlinkF, BlinkMeasure::rFirstBlinkT, BlinkMeasure::rLastNonBlinkT, BlinkMeasure::rLastNonBlinkT-BlinkMeasure::rFirstBlinkT);
                        BlinkMeasure::makeNotification(false);
                        BlinkMeasure::rAdding = false;
                    }
                }
            }
        }
    }
};

Blink::Blink(unsigned int frameStart, unsigned int frameEnd, double timestampStart, double timestampEnd, int eventType) {
    this->frameStart     = frameStart;
    this->frameEnd       = frameEnd;
    this->timestampStart = timestampStart;
    this->timestampEnd   = timestampEnd;
    this->eventType      = eventType;
};

void BlinkMeasure::makeNotification(bool isLeft) {
    if (isLeft == true) {
    } else {
        // right
    }
};
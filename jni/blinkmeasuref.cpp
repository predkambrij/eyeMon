#include <list>
#include <stdio.h>

#include <math.h>
#include <common.hpp>
#include <blinkmeasuref.hpp>


BlinkMeasureF::BlinkMeasureF(unsigned int frameNum, double timestamp, double lcor, double rcor) {
    this->frameNum  = frameNum;
    this->timestamp = timestamp;
    this->lcor      = lcor;
    this->rcor      = rcor;
};

void BlinkMeasureF::measureBlinksAVG(int shortBmSize, double *lavg, double *ravg) {
    std::list<BlinkMeasureF>::iterator iter = blinkMeasureShortf.begin();
    while(iter != blinkMeasureShortf.end()) {
        BlinkMeasureF& bm = *iter;
        *lavg += bm.lcor;
        *ravg += bm.rcor;
        iter++;
    }
    *lavg = *lavg/shortBmSize;
    *ravg = *ravg/shortBmSize;
};

void BlinkMeasureF::measureBlinksSD(int shortBmSize, double lavg, double ravg, double *lSD, double *rSD, double *lsd1, double *rsd1, double *lsd2, double *rsd2) {
    std::list<BlinkMeasureF>::iterator iter = blinkMeasureShortf.begin();
    while(iter != blinkMeasureShortf.end()) {
        BlinkMeasureF& bm = *iter;
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

void BlinkMeasureF::measureBlinks() {
    if (blinkMeasuref.size() == 0) {
        doLog(debug_blinks_d1, "debug_blinks_d1: blinkMeasureSize is zero\n");
        return;
    }

    BlinkMeasureF bm = blinkMeasuref.front();
    blinkMeasuref.pop_front();

    blinkMeasureShortf.push_back(bm);
    int timeWindow = 5;
    while (true) {
        BlinkMeasureF oldestBm = blinkMeasureShortf.front();
        if (oldestBm.timestamp > (bm.timestamp - (timeWindow*1000))) {
            break;
        } else {
            blinkMeasureShortf.pop_front();
        }
    }

    int shortBmSize = blinkMeasureShortf.size();
    if (maxFramesShortList == 0) {
        if (shortBmSize < 30) {
            return;
        }
        BlinkMeasureF first = blinkMeasureShortf.front();
        BlinkMeasureF last = blinkMeasureShortf.back();
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
    BlinkMeasureF::measureBlinksAVG(shortBmSize, &lavg, &ravg);
    double lSD = 0;
    double rSD = 0;
    double lsd1 = 0;
    double rsd1 = 0;
    double lsd2 = 0;
    double rsd2 = 0;
    BlinkMeasureF::measureBlinksSD(shortBmSize, lavg, ravg, &lSD, &rSD, &lsd1, &rsd1, &lsd2, &rsd2);
    doLog(debug_blinks_d1, "debug_blinks_d1: lastF %d T %.2lf La %lf %.8lf Ra %lf %.8lf lSD12 %lf %lf %lf rSD12 %lf %lf %lf\n",
        bm.frameNum, bm.timestamp, bm.lcor, lavg, bm.rcor, ravg, lSD, lsd1, lsd2, rSD, rsd1, rsd2);

    if (bm.lcor < lsd2) {
        doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf L %lf SD1 %lf SD2 %lf\n", bm.frameNum, bm.timestamp, bm.lcor, lsd1, lsd2);
        // check whether we can create a new blink (chunk)
        BlinkMeasureF::makeChunk(true, (double)bm.timestamp, true, bm.frameNum);
    } else {
        BlinkMeasureF::makeChunk(true, (double)bm.timestamp, false, bm.frameNum);
    }
    if (bm.rcor < rsd2) {
        doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf R %lf SD1 %lf SD2 %lf\n", bm.frameNum, bm.timestamp, bm.rcor, rsd1, rsd2);
        // check whether we can create a new blink (chunk)
        BlinkMeasureF::makeChunk(false, (double)bm.timestamp, true, bm.frameNum);
    } else {
        BlinkMeasureF::makeChunk(false, (double)bm.timestamp, false, bm.frameNum);
    }
}

double BlinkMeasureF::maxNonBlinkT = 0.03;
bool BlinkMeasureF::lAdding = false;
bool BlinkMeasureF::rAdding = false;
double BlinkMeasureF::lLastNonBlinkT = -1;
double BlinkMeasureF::rLastNonBlinkT = -1;
unsigned int BlinkMeasureF::lLastNonBlinkF = 0;
unsigned int BlinkMeasureF::rLastNonBlinkF = 0;

double BlinkMeasureF::lFirstBlinkT = 0;
double BlinkMeasureF::rFirstBlinkT = 0;
unsigned int BlinkMeasureF::lFirstBlinkF = 0;
unsigned int BlinkMeasureF::rFirstBlinkF = 0;

void BlinkMeasureF::makeChunk(bool isLeft, double timestamp, bool isBlink, unsigned int frameNum) {
    if (isLeft == true) {
        if (isBlink == true) {
            if (BlinkMeasureF::lAdding == false) {
                BlinkMeasureF::lAdding = true;
                BlinkMeasureF::lFirstBlinkT = timestamp;
                BlinkMeasureF::lFirstBlinkF = frameNum;
            }
            BlinkMeasureF::lLastNonBlinkT = -1;
        } else {
            if (BlinkMeasureF::lAdding == true) {
                if (BlinkMeasureF::lLastNonBlinkT == -1) {
                    BlinkMeasureF::lLastNonBlinkT = timestamp;
                    BlinkMeasureF::lLastNonBlinkF = frameNum;
                } else {
                    if (BlinkMeasureF::lLastNonBlinkT < (timestamp-BlinkMeasureF::maxNonBlinkT)) {
                        BlinkF b(BlinkMeasureF::lFirstBlinkF, BlinkMeasureF::lLastNonBlinkF,
                            BlinkMeasureF::lFirstBlinkT, BlinkMeasureF::lLastNonBlinkT, 0);
                        lBlinkChunksf.push_back(b);
                        doLog(debug_blinks_d4, "debug_blinks_d4: adding_lBlinkChunksf fs %d fe %d start %.2lf end %lf duration %lf\n",
                            BlinkMeasureF::lFirstBlinkF, BlinkMeasureF::lLastNonBlinkF, BlinkMeasureF::lFirstBlinkT, BlinkMeasureF::lLastNonBlinkT, BlinkMeasureF::lLastNonBlinkT-BlinkMeasureF::lFirstBlinkT);
                        BlinkMeasureF::makeNotification(true);
                        BlinkMeasureF::lAdding = false;
                    }
                }
            }
        }
    } else {
        // right
        if (isBlink == true) {
            if (BlinkMeasureF::rAdding == false) {
                BlinkMeasureF::rAdding = true;
                BlinkMeasureF::rFirstBlinkT = timestamp;
                BlinkMeasureF::rFirstBlinkF = frameNum;
            }
            BlinkMeasureF::rLastNonBlinkT = -1;
        } else {
            if (BlinkMeasureF::rAdding == true) {
                if (BlinkMeasureF::rLastNonBlinkT == -1) {
                    BlinkMeasureF::rLastNonBlinkT = timestamp;
                    BlinkMeasureF::rLastNonBlinkF = frameNum;
                } else {
                    if (BlinkMeasureF::rLastNonBlinkT < (timestamp-BlinkMeasureF::maxNonBlinkT)) {
                        BlinkF b(BlinkMeasureF::rFirstBlinkF, BlinkMeasureF::rLastNonBlinkF,
                            BlinkMeasureF::rFirstBlinkT, BlinkMeasureF::rLastNonBlinkT, 0);
                        rBlinkChunksf.push_back(b);
                        doLog(debug_blinks_d4, "debug_blinks_d4: adding_rBlinkChunksf fs %d fe %d start %.2lf end %lf duration %lf\n",
                            BlinkMeasureF::rFirstBlinkF, BlinkMeasureF::rLastNonBlinkF, BlinkMeasureF::rFirstBlinkT, BlinkMeasureF::rLastNonBlinkT, BlinkMeasureF::rLastNonBlinkT-BlinkMeasureF::rFirstBlinkT);
                        BlinkMeasureF::makeNotification(false);
                        BlinkMeasureF::rAdding = false;
                    }
                }
            }
        }
    }
};

BlinkF::BlinkF(unsigned int frameStart, unsigned int frameEnd, double timestampStart, double timestampEnd, int eventType) {
    this->frameStart     = frameStart;
    this->frameEnd       = frameEnd;
    this->timestampStart = timestampStart;
    this->timestampEnd   = timestampEnd;
    this->eventType      = eventType;
};

void BlinkMeasureF::makeNotification(bool isLeft) {
    if (isLeft == true) {
    } else {
        // right
    }
};

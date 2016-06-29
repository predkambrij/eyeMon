#include <list>
#include <stdio.h>

#include <opencv2/core/core.hpp>

#include <math.h>
#include <common.hpp>
#include <blinkmeasuref.hpp>


BlinkMeasureF::BlinkMeasureF(unsigned int frameNum, double timestamp, cv::Point2d lDiffP, cv::Point2d rDiffP, bool canProceedL, bool canProceedR, bool canUpdateL, bool canUpdateR) {
    this->frameNum  = frameNum;
    this->timestamp = timestamp;
    this->lDiffP    = lDiffP;
    this->rDiffP    = rDiffP;
    this->canProceedL = canProceedL;
    this->canProceedR = canProceedR;
    this->canUpdateL = canUpdateL;
    this->canUpdateR = canUpdateR;
};

void BlinkMeasureF::measureBlinksAVG(double *lavg, double *ravg) {
    int lSize = 0, rSize = 0;
    std::list<BlinkMeasureF>::iterator iter = blinkMeasureShortf.begin();
    while(iter != blinkMeasureShortf.end()) {
        BlinkMeasureF& bm = *iter;
        if (bm.canProceedL == true) {
            *lavg += bm.lDiffP.y;
            lSize++;
        }
        if (bm.canProceedR == true) {
            *ravg += bm.rDiffP.y;
            rSize++;
        }
        iter++;
    }
    if (lSize > 0) {
        *lavg = *lavg/lSize;
    } else {
        *lavg = 0;
    }
    if (rSize > 0) {
        *ravg = *ravg/rSize;
    } else {
        *ravg = 0;
    }
};

void BlinkMeasureF::measureBlinksSD(double lavg, double ravg, double *lSD, double *rSD, double *plsd1, double *prsd1, double *plsd2, double *prsd2, double *plsdt, double *prsdt, double *mlsd1, double *mrsd1, double *mlsd2, double *mrsd2, double *mlsdt, double *mrsdt) {
    int lSize = 0, rSize = 0;
    std::list<BlinkMeasureF>::iterator iter = blinkMeasureShortf.begin();
    while(iter != blinkMeasureShortf.end()) {
        BlinkMeasureF& bm = *iter;
        if (bm.canProceedL == true) {
            *lSD = *lSD+pow(lavg-bm.lDiffP.y, 2);
            lSize++;
        }
        if (bm.canProceedR == true) {
            *rSD = *rSD+pow(ravg-bm.rDiffP.y, 2);
            rSize++;
        }
        iter++;
    }
    if (lSize > 0) {
        *lSD = pow(*lSD/lSize, 0.5);
    } else {
        *lSD = 0;
    }
    if (rSize > 0) {
        *rSD = pow(*rSD/rSize, 0.5);
    } else {
        *rSD = 0;
    }
    *plsd1 = lavg+(1*(*lSD));
    *prsd1 = ravg+(1*(*rSD));
    *plsd2 = lavg+(2*(*lSD));
    *prsd2 = ravg+(2*(*rSD));
    *plsdt = lavg+(3*(*lSD));
    *prsdt = ravg+(3*(*rSD));
    *mlsd1 = lavg-(1*(*lSD));
    *mrsd1 = ravg-(1*(*rSD));
    *mlsd2 = lavg-(2*(*lSD));
    *mrsd2 = ravg-(2*(*rSD));
    *mlsdt = lavg-(3*(*lSD));
    *mrsdt = ravg-(3*(*rSD));
};
int    BlinkMeasureF::lCurState = 0;
double BlinkMeasureF::lLastVal = 0;
int    BlinkMeasureF::lZeroCrossPosToNegF = 0;
double BlinkMeasureF::lZeroCrossPosToNegT = 0;
int    BlinkMeasureF::rCurState = 0;
double BlinkMeasureF::rLastVal = 0;
int    BlinkMeasureF::rZeroCrossPosToNegF = 0;
double BlinkMeasureF::rZeroCrossPosToNegT = 0;

void BlinkMeasureF::measureBlinks(BlinkMeasureF bm) {
    blinkMeasureShortf.push_back(bm);
    int timeWindow = 15;
    while (true) {
        BlinkMeasureF oldestBm = blinkMeasureShortf.front();
        if (oldestBm.timestamp > (bm.timestamp - (timeWindow*1000))) {
            break;
        } else {
            blinkMeasureShortf.pop_front();
        }
    }

    int firstMeasureQueueSize = 30;
    int shortBmSize = blinkMeasureShortf.size();
    if (maxFramesShortList == 0) {
        if (shortBmSize < firstMeasureQueueSize) {
            return;
        }
        BlinkMeasureF first = blinkMeasureShortf.front();
        BlinkMeasureF last = blinkMeasureShortf.back();
        double tsDiff = last.timestamp-first.timestamp;
        double fps = shortBmSize/(tsDiff/1000);
        maxFramesShortList = fps*timeWindow*0.80;
        doLog(debug_blinks_d2, "debug_blinks_d2: F %d fps of the first %d frames %lf current maxFramesShortList %d\n",
            bm.frameNum, firstMeasureQueueSize, fps, maxFramesShortList);
    } else {
        if (shortBmSize > maxFramesShortList) {
            maxFramesShortList = shortBmSize;
            doLog(debug_blinks_d2, "debug_blinks_d2: updated maxFramesShortList %d\n", maxFramesShortList);
        }
    }
    if (shortBmSize < (maxFramesShortList/2)) {
        doLog(debug_blinks_d2, "debug_blinks_d2: F %d shortBmSize is less than max/2 %d T %lf\n", bm.frameNum, shortBmSize, bm.timestamp);
        return;
    } else {
        doLog(debug_blinks_d2, "debug_blinks_d2: F %d shortBmSize is big enough %d\n", bm.frameNum, shortBmSize);
    }

    double lavg = 0;
    double ravg = 0;
    BlinkMeasureF::measureBlinksAVG(&lavg, &ravg);
    double lSD = 0, rSD = 0;
    double plsd1 = 0, prsd1 = 0, mlsd1 = 0, mrsd1 = 0;
    double plsd2 = 0, prsd2 = 0, mlsd2 = 0, mrsd2 = 0;
    double plsdt = 0, prsdt = 0, mlsdt = 0, mrsdt = 0;
    BlinkMeasureF::measureBlinksSD(lavg, ravg, &lSD, &rSD, &plsd1, &prsd1, &plsd2, &prsd2, &plsdt, &prsdt, &mlsd1, &mrsd1, &mlsd2, &mrsd2, &mlsdt, &mrsdt);
    if (bm.canProceedL == true && bm.canProceedR == true) {
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d T %.2lf logType b La %lf %.8lf Ra %lf %.8lf lrSD %lf %lf plrSD12t %lf %lf %lf %lf %lf %lf mlrSD12t %lf %lf %lf %lf %lf %lf\n",
            bm.frameNum, bm.timestamp, bm.lDiffP.y, lavg, bm.rDiffP.y, ravg, lSD, rSD, plsd1, plsd2, plsdt, prsd1, prsd2, prsdt, mlsd1, mlsd2, mlsdt, mrsd1, mrsd2, mrsdt);
    } else if (bm.canProceedL == true && bm.canProceedR == false) {
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d T %.2lf logType l La %lf %.8lf lrSD %lf plrSD12t %lf %lf %lf mlrSD12t %lf %lf %lf\n",
            bm.frameNum, bm.timestamp, bm.lDiffP.y, lavg, lSD, plsd1, plsd2, plsdt, mlsd1, mlsd2, mlsdt);
    } else if (bm.canProceedL == false && bm.canProceedR == true) {
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d T %.2lf logType r Ra %lf %.8lf lrSD %lf plrSD12t %lf %lf %lf mlrSD12t %lf %lf %lf\n",
            bm.frameNum, bm.timestamp, bm.rDiffP.y, ravg, rSD, prsd1, prsd2, prsdt, mrsd1, mrsd2, mrsdt);
    } else if (bm.canProceedL == false && bm.canProceedR == false) {
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d T %.2lf logType n\n", bm.frameNum, bm.timestamp);
    }

    BlinkMeasureF::stateMachine(bm.frameNum, bm.timestamp, bm.lDiffP.y, mlsd2, plsd2, bm.rDiffP.y, mrsd2, prsd2);
    //BlinkMeasureF::stateMachine(bm.frameNum, bm.timestamp, bm.lDiffP.y, mlsdt, plsdt, bm.rDiffP.y, mrsdt, prsdt);

    // if (bm.lDiffP.y < lsd2) {
    //     doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf L %lf SD1 %lf SD2 %lf\n", bm.frameNum, bm.timestamp, bm.lDiffP.y, lsd1, lsd2);
    //     // check whether we can create a new blink (chunk)
    //     BlinkMeasureF::makeChunk(true, (double)bm.timestamp, true, bm.frameNum);
    // } else {
    //     BlinkMeasureF::makeChunk(true, (double)bm.timestamp, false, bm.frameNum);
    // }
    // if (bm.rDiffP.y < rsd2) {
    //     doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf R %lf SD1 %lf SD2 %lf\n", bm.frameNum, bm.timestamp, bm.rDiffP.y, rsd1, rsd2);
    //     // check whether we can create a new blink (chunk)
    //     BlinkMeasureF::makeChunk(false, (double)bm.timestamp, true, bm.frameNum);
    // } else {
    //     BlinkMeasureF::makeChunk(false, (double)bm.timestamp, false, bm.frameNum);
    // }
}

void BlinkMeasureF::stateMachine(unsigned int frameNum, double timestamp, double leftY, double leftLowSD, double leftHighSD, double rightY, double rightLowSD, double rightHighSD) {
    if (BlinkMeasureF::lCurState == 0) {
        if (BlinkMeasureF::lLastVal == 0) {
            if (leftY == 0) {
                return;
            }
            // first loop only
            BlinkMeasureF::lLastVal = leftY;
        }
        if (BlinkMeasureF::lLastVal > 0 && leftY < 0) {
            BlinkMeasureF::lZeroCrossPosToNegF = frameNum;
            BlinkMeasureF::lZeroCrossPosToNegT = timestamp;
        }
        if (leftY < leftLowSD) {
            BlinkMeasureF::lCurState = 1;
        }
    } else if (BlinkMeasureF::lCurState == 1) {
        if ((timestamp-BlinkMeasureF::lZeroCrossPosToNegT) > 500) {
            BlinkMeasureF::lCurState = 0;
        } else {
            if (leftY > leftHighSD) {
                BlinkMeasureF::lCurState = 2;
            }
        }
    } else if (BlinkMeasureF::lCurState == 2) {
        if ((timestamp-BlinkMeasureF::lZeroCrossPosToNegT) > 500) {
            BlinkMeasureF::lCurState = 0;
        } else {
            if (BlinkMeasureF::lLastVal > 0 && leftY < 0) {
                BlinkF b(BlinkMeasureF::lZeroCrossPosToNegF, frameNum, BlinkMeasureF::lZeroCrossPosToNegT, timestamp, 0);
                lBlinkChunksf.push_back(b);
                doLog(debug_blinks_d4, "debug_blinks_d4: adding_lBlinkChunksf fs %d fe %d start %.2lf end %lf duration %lf\n",
                    BlinkMeasureF::lZeroCrossPosToNegF, frameNum, BlinkMeasureF::lZeroCrossPosToNegT, timestamp, timestamp-BlinkMeasureF::lZeroCrossPosToNegT);

                BlinkMeasureF::lZeroCrossPosToNegF = frameNum;
                BlinkMeasureF::lZeroCrossPosToNegT = timestamp;
                BlinkMeasureF::lCurState = 0;
            }
        }
    }
    // update last value
    if (leftY != 0) {
        BlinkMeasureF::lLastVal = leftY;
    }
    //
    if (BlinkMeasureF::rCurState == 0) {
        if (BlinkMeasureF::rLastVal == 0) {
            if (rightY == 0) {
                return;
            }
            // first loop only
            BlinkMeasureF::rLastVal = rightY;
        }
        if (BlinkMeasureF::rLastVal > 0 && rightY < 0) {
            BlinkMeasureF::rZeroCrossPosToNegF = frameNum;
            BlinkMeasureF::rZeroCrossPosToNegT = timestamp;
        }
        if (rightY < rightLowSD) {
            BlinkMeasureF::rCurState = 1;
        }
    } else if (BlinkMeasureF::rCurState == 1) {
        if ((timestamp-BlinkMeasureF::rZeroCrossPosToNegT) > 500) {
            BlinkMeasureF::rCurState = 0;
        } else {
            if (rightY > rightHighSD) {
                BlinkMeasureF::rCurState = 2;
            }
        }
    } else if (BlinkMeasureF::rCurState == 2) {
        if ((timestamp-BlinkMeasureF::rZeroCrossPosToNegT) > 500) {
            BlinkMeasureF::rCurState = 0;
        } else {
            if (BlinkMeasureF::rLastVal > 0 && rightY < 0) {
                BlinkF b(BlinkMeasureF::rZeroCrossPosToNegF, frameNum, BlinkMeasureF::rZeroCrossPosToNegT, timestamp, 0);
                rBlinkChunksf.push_back(b);
                doLog(debug_blinks_d4, "debug_blinks_d4: adding_rBlinkChunksf fs %d fe %d start %.2lf end %lf duration %lf\n",
                    BlinkMeasureF::rZeroCrossPosToNegF, frameNum, BlinkMeasureF::rZeroCrossPosToNegT, timestamp, timestamp-BlinkMeasureF::rZeroCrossPosToNegT);

                BlinkMeasureF::rZeroCrossPosToNegF = frameNum;
                BlinkMeasureF::rZeroCrossPosToNegT = timestamp;
                BlinkMeasureF::rCurState = 0;
            }
        }
    }

    // update last value
    if (rightY != 0) {
        BlinkMeasureF::rLastVal = rightY;
    }
    return;
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

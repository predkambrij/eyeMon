#include <list>
#include <stdio.h>

#include <math.h>
#include <set>
#include <common.hpp>
#include <blinkmeasure.hpp>

BlinkMeasure::BlinkMeasure() {
}
BlinkMeasure::BlinkMeasure(unsigned int frameNum, double timestamp, double lcor, double rcor) {
    this->frameNum  = frameNum;
    this->timestamp = timestamp;
    this->lcor      = lcor;
    this->rcor      = rcor;
    this->canProceedL = true;
    this->canProceedR = true;
    this->canUpdateL = true;
    this->canUpdateR = true;
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

void BlinkMeasure::measureBlinksSD(double *lSD, double *rSD, double *lsdt, double *rsdt, double *plsdf, double *prsdf, double *mlsdf, double *mrsdf) {
    int lSize, rSize;
    double lavg, ravg;
    double prevValL, prevValR;
    int i;
    std::list<BlinkMeasure>::iterator iter;

    // mean for first SD
    lavg = 0; ravg = 0;
    lSize = 0; rSize = 0;
    iter = blinkMeasureShort.begin();
    i = 0;
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        if (bm.canProceedL == true) {
            if (i == 0) {
                prevValL = bm.lcor;
            } else {
                lavg += (bm.lcor-prevValL);
                prevValL = bm.lcor;
                lSize++;
            }
        }
        if (bm.canProceedR == true) {
            if (i == 0) {
                prevValR = bm.rcor;
            } else {
                ravg += (bm.rcor-prevValR);
                prevValR = bm.rcor;
                rSize++;
            }
        }
        iter++;
        i++;
    }
    if (lSize > 0) {
        lavg = lavg/lSize;
    } else {
        lavg = 0;
    }
    if (rSize > 0) {
        ravg = ravg/rSize;
    } else {
        ravg = 0;
    }

    // measure SD for defining the second one
    lSize = 0; rSize = 0;
    iter = blinkMeasureShort.begin();
    i = 0;
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        if (bm.canProceedL == true) {
            if (i == 0) {
                prevValL = bm.lcor;
            } else {
                *lSD = *lSD+pow(lavg-(bm.lcor-prevValL), 2);
                prevValL = bm.lcor;
                lSize++;
            }
        }
        if (bm.canProceedR == true) {
            if (i == 0) {
                prevValR = bm.rcor;
            } else {
                *rSD = *rSD+pow(ravg-(bm.rcor-prevValR), 2);
                prevValR = bm.rcor;
                rSize++;
            }
        }
        iter++;
        i++;
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
    double fSDMag = 4;
    *plsdf = lavg+(fSDMag*(*lSD));
    *prsdf = ravg+(fSDMag*(*rSD));
    *mlsdf = lavg-(fSDMag*(*lSD));
    *mrsdf = ravg-(fSDMag*(*rSD));

    double doubleSD = true;
    if (doubleSD == true) {
        // second SD
        double plTmpSDf = lavg+(fSDMag*(*lSD));
        double prTmpSDf = ravg+(fSDMag*(*rSD));
        double mlTmpSDf = lavg-(fSDMag*(*lSD));
        double mrTmpSDf = ravg-(fSDMag*(*rSD));

        // mean for second SD
        lavg = 0; ravg = 0;
        lSize = 0; rSize = 0;
        iter = blinkMeasureShort.begin();
        i=0;
        while(iter != blinkMeasureShort.end()) {
            BlinkMeasure& bm = *iter;
            if (bm.canProceedL == true) {
                if (i == 0) {
                    prevValL = bm.lcor;
                } else {
                    if (mlTmpSDf < (bm.lcor-prevValL) && (bm.lcor-prevValL) < plTmpSDf) {
                        lavg += (bm.lcor-prevValL);
                        prevValL = bm.lcor;
                        lSize++;
                    } else {
                        prevValL = bm.lcor;
                    }
                }
            }
            if (bm.canProceedR == true) {
                if (i == 0) {
                    prevValR = bm.rcor;
                } else {
                    if (mrTmpSDf < (bm.rcor-prevValR) && (bm.rcor-prevValR) < prTmpSDf) {
                        ravg += (bm.rcor-prevValR);
                        prevValR = bm.rcor;
                        rSize++;
                    } else {
                        prevValR = bm.rcor;
                    }
                }
            }
            iter++;
            i++;
        }
        if (lSize > 0) {
            lavg = lavg/lSize;
        } else {
            lavg = 0;
        }
        if (rSize > 0) {
            ravg = ravg/rSize;
        } else {
            ravg = 0;
        }

        // second
        *lSD = 0;
        *rSD = 0;
        lSize = 0, rSize = 0;
        iter = blinkMeasureShort.begin();
        i=0;
        while(iter != blinkMeasureShort.end()) {
            BlinkMeasure& bm = *iter;
            if (bm.canProceedL == true) {
                if (i == 0) {
                    prevValL = bm.lcor;
                } else {
                    if (mlTmpSDf < (bm.lcor-prevValL) && (bm.lcor-prevValL) < plTmpSDf) {
                        *lSD = *lSD+pow(lavg-(bm.lcor-prevValL), 2);
                        prevValL = bm.lcor;
                        lSize++;
                    } else {
                        prevValL = bm.lcor;
                    }
                }
            }
            if (bm.canProceedR == true) {
                if (i == 0) {
                    prevValR = bm.rcor;
                } else {
                    if (mrTmpSDf < (bm.rcor-prevValR) && (bm.rcor-prevValR) < prTmpSDf) {
                        *rSD = *rSD+pow(ravg-(bm.rcor-prevValR), 2);
                        prevValR = bm.rcor;
                        rSize++;
                    } else {
                        prevValR = bm.rcor;
                    }
                }
            }
            iter++;
            i++;
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
    }
    *lsdt = (*lSD)*5;
    *rsdt = (*rSD)*5;
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

    double lavg = 0, ravg = 0;
    BlinkMeasure::measureBlinksAVG(shortBmSize, &lavg, &ravg);
    double lSD = 0, rSD = 0;
    double lsdt = 0, rsdt = 0;
    double plsdf = 0, prsdf = 0, mlsdf = 0, mrsdf = 0;
    BlinkMeasure::measureBlinksSD(&lSD, &rSD, &lsdt, &rsdt, &plsdf, &prsdf, &mlsdf, &mrsdf);
    if (BlinkMeasure::isFirst == false) {
        BlinkMeasure::isFirst = true;
        BlinkMeasure::prevLcor = bm.lcor;
        BlinkMeasure::prevRcor = bm.rcor;
    } else {
        doLog(debug_blinks_d1, "debug_blinks_d1: lastF %d T %.2lf La %lf %lf %.8lf Ra %lf %lf %.8lf lSDft %lf %lf %lf %lf rSDft %lf %lf %lf %lf\n",
            bm.frameNum, bm.timestamp,
            bm.lcor, lavg, (bm.lcor-BlinkMeasure::prevLcor), bm.rcor, ravg, (bm.rcor-BlinkMeasure::prevRcor),
            lSD, plsdf, mlsdf, lsdt, rSD, prsdf, mrsdf, rsdt);
            BlinkMeasure::prevLcor = bm.lcor;
            BlinkMeasure::prevRcor = bm.rcor;
    }

    // spike detection
    BlinkMeasure spikeEndBm = blinkMeasureShort.back();
    BlinkMeasure spikeStartBm;
    //double maxTimediffBetweenFrames = 150;
    int skipElements = shortBmSize-(maxFramesShortList/(timeWindow*2));
    if (skipElements < 0) {
        skipElements = 0;
    }
    int i=0;
    std::list<BlinkMeasure>::iterator iter = blinkMeasureShort.begin();
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        if (i == skipElements) {
            // frames might repeat because maxFramesShortList might extend
            // preventing that by using (l/r)LastAddedFN
            if (bm.frameNum > BlinkMeasure::lLastAddedFN) {
                if (toChunksLeft.find(bm.frameNum) != toChunksLeft.end()) {
                    doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf L %lf\n", bm.frameNum, bm.timestamp, bm.lcor);
                    BlinkMeasure::makeChunk(true, bm.timestamp, true, bm.frameNum);
                    toChunksLeft.erase (bm.frameNum);
                } else {
                    BlinkMeasure::makeChunk(true, bm.timestamp, false, bm.frameNum);
                }
                BlinkMeasure::lLastAddedFN = bm.frameNum;
            }
            if (bm.frameNum > BlinkMeasure::rLastAddedFN) {
                if (toChunksRight.find(bm.frameNum) != toChunksRight.end()) {
                    doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf R %lf\n", bm.frameNum, bm.timestamp, bm.rcor);
                    BlinkMeasure::makeChunk(false, bm.timestamp, true, bm.frameNum);
                    toChunksLeft.erase (bm.frameNum);
                } else {
                    BlinkMeasure::makeChunk(false, bm.timestamp, false, bm.frameNum);
                }
                BlinkMeasure::rLastAddedFN = bm.frameNum;
            }
            spikeStartBm = bm;
        }
        if (i > skipElements) {
            if (bm.lcor < (spikeStartBm.lcor-lsdt) && bm.lcor < (spikeEndBm.lcor-lsdt)) {
                toChunksLeft[bm.frameNum] = bm.timestamp;
            }
            if (bm.rcor < (spikeStartBm.rcor-rsdt) && bm.rcor < (spikeEndBm.rcor-rsdt)) {
                toChunksRight[bm.frameNum] = bm.timestamp;
            }
        }
        i++;
        iter++;
    }

/*
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
*/
}

double BlinkMeasure::maxNonBlinkT = 0.03;
bool BlinkMeasure::lAdding = false;
bool BlinkMeasure::rAdding = false;
bool BlinkMeasure::isFirst = false;
double BlinkMeasure::prevLcor = 0;
double BlinkMeasure::prevRcor = 0;
double BlinkMeasure::lLastNonBlinkT = -1;
double BlinkMeasure::rLastNonBlinkT = -1;
unsigned int BlinkMeasure::lLastNonBlinkF = 0;
unsigned int BlinkMeasure::rLastNonBlinkF = 0;

unsigned int BlinkMeasure::lLastAddedFN = 0;
unsigned int BlinkMeasure::rLastAddedFN = 0;

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


bool BlinkMeasure::joinBlinks() {
    bool anyAdded = false;
    if (BlinkMeasure::lAdding == true || BlinkMeasure::rAdding == true) {
        // waiting that blinks from both eyes are finished
        return false;
    }
    std::set<unsigned int> takenRBlinks;

    std::list<Blink>::iterator lIter = lBlinkChunks.begin();
    while(lIter != lBlinkChunks.end()) {
        Blink& lb = *lIter;

        std::list<Blink>::iterator rIter = rBlinkChunks.begin();
        while(rIter != rBlinkChunks.end()) {
            Blink& rb = *rIter;
            if (takenRBlinks.find(rb.frameStart) != takenRBlinks.end()) {
                rIter++;
                continue;
            }
            if ((lb.frameStart <= rb.frameStart && rb.frameStart <= lb.frameEnd)
                    || (lb.frameStart <= rb.frameEnd && rb.frameEnd <= lb.frameEnd)
                    || (rb.frameStart <= lb.frameStart && lb.frameStart <= rb.frameEnd)
                    || (rb.frameStart <= lb.frameEnd && lb.frameEnd <= rb.frameEnd)) {
                // use extended length as a result (joined) blink
                unsigned int frameStart = (lb.frameStart < rb.frameStart)?lb.frameStart:rb.frameStart;
                unsigned int frameEnd = (lb.frameEnd > rb.frameEnd)?lb.frameEnd:rb.frameEnd;
                double timestampStart = (lb.timestampStart < rb.timestampStart)?lb.timestampStart:rb.timestampStart;
                double timestampEnd = (lb.timestampEnd > rb.timestampEnd)?lb.timestampEnd:rb.timestampEnd;
                Blink joinedBlink(frameStart, frameEnd, timestampStart, timestampEnd, 0);
                joinedBlinkChunks.push_back(joinedBlink);
                anyAdded = true;
                takenRBlinks.insert(rb.frameStart);
                doLog(debug_blinks_d5, "debug_blinks_d5: added %u-%u (%.3lf)\n", frameStart, frameEnd, timestampEnd-timestampStart);
                break;
            }
            rIter++;
        }
        lIter++;
    }

    lBlinkChunks.clear();
    rBlinkChunks.clear();
    return anyAdded;
}

void BlinkMeasure::makeNotification(bool isLeft) {
    if (isLeft == true) {
    } else {
        // right
    }
};
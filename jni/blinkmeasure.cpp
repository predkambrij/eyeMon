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

void BlinkMeasure::measureBlinksAVG(double *lavg, double *ravg) {
    std::list<BlinkMeasure>::iterator iter = blinkMeasureShort.begin();
    int shortBmSize = 0;
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bm = *iter;
        *lavg += bm.lcor;
        *ravg += bm.rcor;
        iter++;
        shortBmSize++;
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

bool BlinkMeasure::measureBlinks(BlinkMeasure bm) {
    int timeWindow = 10;

    // ensure that queue is long enough (at least 30 frames) (that we'll know how long we want to have it)
    int firstMeasureQueueSize = 30;
    int shortBmSize = blinkMeasureShort.size();
    if (maxFramesShortList == 0) {
        if (shortBmSize < firstMeasureQueueSize) {
            blinkMeasureShort.push_back(bm);
            return false;
        }
        BlinkMeasure first = blinkMeasureShort.front();
        BlinkMeasure last = blinkMeasureShort.back();
        double tsDiff = last.timestamp-first.timestamp;
        double fps = shortBmSize/(tsDiff/1000);
        maxFramesShortList = fps*timeWindow*0.80;
        doLog(debug_blinks_d1, "debug_blinks_d1: F %d fps of the first %d frames %lf current maxFramesShortList %d\n",
            bm.frameNum, firstMeasureQueueSize, fps, maxFramesShortList);
    } else {
        if (shortBmSize > maxFramesShortList) {
            maxFramesShortList = shortBmSize;
            doLog(debug_blinks_d1, "debug_blinks_d1: F %d updated maxFramesShortList %d\n", bm.frameNum, maxFramesShortList);
        }
    }

    bool rewriteElementsToStateQueue = false;
    int minShortBmSize = maxFramesShortList/2;
    while (true && shortBmSize > 1) {
        BlinkMeasure oldestBm = blinkMeasureShort.front();
        if (oldestBm.timestamp > (bm.timestamp - (timeWindow*1000))) {
            break;
        } else {
            if (BlinkMeasure::delayStateMachine == true) {
                if (oldestBm.frameNum > BlinkMeasure::lastAddedToStateMachine
                    || (oldestBm.frameNum == 0 && BlinkMeasure::lastAddedToStateMachine == 0)) {
                    // reliable shortsize length?
                    if (shortBmSize < minShortBmSize) {
                        blinkMeasureShort.pop_front();
                        doLog(debug_blinks_d2, "debug_blinks_d2: F %d T %lf shortBmSize %d is less than minShortBmSize %d (skipping %d)\n",
                            bm.frameNum, bm.timestamp, shortBmSize, minShortBmSize, oldestBm.frameNum);
                    } else {
                        // reliable length, don't delete it, write it to state machine queue first!
                        rewriteElementsToStateQueue = true;
                        break;
                    }
                } else {
                    blinkMeasureShort.pop_front();
                }
            } else {
                blinkMeasureShort.pop_front();
            }
        }
        shortBmSize = blinkMeasureShort.size();
    }
    if (BlinkMeasure::delayStateMachine == false) {
        if (shortBmSize < minShortBmSize) {
            BlinkMeasure::delayStateMachine = true;
        } else {
            BlinkMeasure prevBm = blinkMeasureShort.back();
            double lavg = 0, ravg = 0, lSD = 0, rSD = 0, lsdt = 0, rsdt = 0, plsdf = 0, prsdf = 0, mlsdf = 0, mrsdf = 0;
            BlinkMeasure::measureBlinksAVG(&lavg, &ravg);
            BlinkMeasure::measureBlinksSD(&lSD, &rSD, &lsdt, &rsdt, &plsdf, &prsdf, &mlsdf, &mrsdf);
            BlinkMeasure::processBm(bm, lavg, ravg, lSD, rSD, plsdf, mlsdf, prsdf, mrsdf, lsdt, rsdt);

            stateMachineElementT e; e.bm = bm; e.lsdt = lsdt; e.rsdt = rsdt;
            doLog(debug_blinks_d2, "debug_blinks_d2: F %d T %lf pushing %u to stateMachineQueue\n", bm.frameNum, bm.timestamp, prevBm.frameNum);
            stateMachineQueueT.push_back(e);
            BlinkMeasure::lastAddedToStateMachine = prevBm.frameNum;
        }
    }

    if (rewriteElementsToStateQueue == true) {
        // happens only if delayStateMachine is/was true
        BlinkMeasure::delayStateMachine = false;
        BlinkMeasure::rewriteElementsToStateQueue(bm.frameNum, bm.timestamp);
    }

    blinkMeasureShort.push_back(bm);

    return (rewriteElementsToStateQueue == false && BlinkMeasure::delayStateMachine == false);
}
void BlinkMeasure::rewriteElementsToStateQueue(unsigned int frameNum, double timestamp) {
    double lavg = 0, ravg = 0, lSD = 0, rSD = 0, lsdt = 0, rsdt = 0, plsdf = 0, prsdf = 0, mlsdf = 0, mrsdf = 0;
    BlinkMeasure::measureBlinksAVG(&lavg, &ravg);
    BlinkMeasure::measureBlinksSD(&lSD, &rSD, &lsdt, &rsdt, &plsdf, &prsdf, &mlsdf, &mrsdf);

    std::list<BlinkMeasure>::iterator iter;
    iter = blinkMeasureShort.begin();
    while(iter != blinkMeasureShort.end()) {
        BlinkMeasure& bmItem = *iter;
        BlinkMeasure::processBm(bmItem, lavg, ravg, lSD, rSD, plsdf, mlsdf, prsdf, mrsdf, lsdt, rsdt);

        stateMachineElementT e; e.bm = bmItem; e.lsdt = lsdt; e.rsdt = rsdt;
        doLog(debug_blinks_d2, "debug_blinks_d2: F %d T %lf pushing %u to stateMachineQueue\n", frameNum, timestamp, bmItem.frameNum);
        stateMachineQueueT.push_back(e);
        BlinkMeasure::lastAddedToStateMachine = bmItem.frameNum;

        iter++;
    }
}
void BlinkMeasure::processBm(BlinkMeasure bm, double lavg, double ravg, double lSD, double rSD,
        double plsdf, double mlsdf, double prsdf, double mrsdf, double lsdt, double rsdt) {
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
}

void BlinkMeasure::processStateMachineQueue() {
    int timeWindow = 10;
    int watchingWindowLength = maxFramesShortList/(timeWindow*2); // meant to be up to 500ms long
    if (watchingWindowLength < 3) {
        return;
    }
    int stateMachineQueueTSize = stateMachineQueueT.size();
    while (stateMachineQueueTSize >= watchingWindowLength) {
        // spike start
        stateMachineElementT spikeStart = stateMachineQueueT.front();
        BlinkMeasure spikeStartBm = spikeStart.bm;

        // find spike ending
        stateMachineElementT spikeEnd;
        BlinkMeasure spikeEndBm;
        double lsdt, rsdt;
        int position = 0;
        std::list<stateMachineElementT>::iterator iterS;
        iterS = stateMachineQueueT.begin();
        while(iterS != stateMachineQueueT.end()) {
            position++;
            if (position == watchingWindowLength) {
                stateMachineElementT& sme = *iterS;
                spikeEnd = sme;
                spikeEndBm = spikeEnd.bm;
                break;
            }
            iterS++;
        }

        // write detected spikes
        if (toChunksLeft.find(spikeStartBm.frameNum) != toChunksLeft.end()) {
            doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf L %lf\n", spikeStartBm.frameNum, spikeStartBm.timestamp, spikeStartBm.lcor);
            BlinkMeasure::makeChunk(true, spikeStartBm.timestamp, true, spikeStartBm.frameNum);
            toChunksLeft.erase (spikeStartBm.frameNum);
        } else {
            BlinkMeasure::makeChunk(true, spikeStartBm.timestamp, false, spikeStartBm.frameNum);
        }
        if (toChunksRight.find(spikeStartBm.frameNum) != toChunksRight.end()) {
            doLog(debug_blinks_d3, "debug_blinks_d3: BLINK F %d T %.2lf R %lf\n", spikeStartBm.frameNum, spikeStartBm.timestamp, spikeStartBm.rcor);
            BlinkMeasure::makeChunk(false, spikeStartBm.timestamp, true, spikeStartBm.frameNum);
            toChunksLeft.erase (spikeStartBm.frameNum);
        } else {
            BlinkMeasure::makeChunk(false, spikeStartBm.timestamp, false, spikeStartBm.frameNum);
        }

        if ((spikeEndBm.timestamp-spikeStartBm.timestamp) > 800) {
            doLog(debug_blinks_d2, "debug_blinks_d2: length %d too long %.2f\n", watchingWindowLength, spikeEndBm.timestamp-spikeStartBm.timestamp);
            stateMachineQueueT.pop_front();
            return;
        } else {
            doLog(debug_blinks_d2, "debug_blinks_d2: length %d %.2f\n", watchingWindowLength, spikeEndBm.timestamp-spikeStartBm.timestamp);
        }

        // process the window
        lsdt = spikeEnd.lsdt; rsdt = spikeEnd.rsdt;
        iterS = stateMachineQueueT.begin();
        while(iterS != stateMachineQueueT.end()) {
            stateMachineElementT& sme = *iterS;

            if (sme.bm.lcor < (spikeStartBm.lcor-lsdt) && sme.bm.lcor < (spikeEndBm.lcor-lsdt)) {
                toChunksLeft[sme.bm.frameNum] = sme.bm.timestamp;
            }
            if (sme.bm.rcor < (spikeStartBm.rcor-rsdt) && sme.bm.rcor < (spikeEndBm.rcor-rsdt)) {
                toChunksRight[sme.bm.frameNum] = sme.bm.timestamp;
            }

            if (sme.bm.frameNum == spikeEndBm.frameNum) {
                break;
            }
            iterS++;
        }

        // track watching time for notifications
        if (BlinkMeasure::prevTS == -1) {
            BlinkMeasure::prevTS = spikeStartBm.timestamp;
            BlinkMeasure::startTS = spikeStartBm.timestamp;
        } else {
            if ((spikeStartBm.timestamp - BlinkMeasure::prevTS) > 200) {
                activeSlice as;
                as.start = BlinkMeasure::startTS;
                as.end = BlinkMeasure::prevTS;
                n1ActiveSlices.push_back(as);
                BlinkMeasure::startTS = spikeStartBm.timestamp;
            }
            BlinkMeasure::prevTS = spikeStartBm.timestamp;
        }

        stateMachineQueueT.pop_front();
        stateMachineQueueTSize = stateMachineQueueT.size();
    }
}

bool BlinkMeasure::checkN1Notifs(double curTimestamp) {
    // 5 mins
    //double watchingWindow = 1000*60*5;
    double watchingWindow = 1000*10;
    double minBlinksRatio = 12/(double)60;
    double winStart = curTimestamp-watchingWindow;
    double minWindowLength = watchingWindow/2;
    double watchedLength = 0;

    std::list<struct activeSlice>::iterator iter;

    iter = n1ActiveSlices.begin();
    while(iter != n1ActiveSlices.end()) {
        struct activeSlice& as = *iter;
        //doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1 s %.2f e %.2f d %.2f\n", as.start, as.end, as.end-as.start);
        if (as.end < winStart) {
            //n1ActiveSlices.pop_front();
            iter = n1ActiveSlices.erase(iter);
            continue;
        } else {
            if (as.start < winStart) {
                watchedLength += (as.end-winStart);
            } else {
                watchedLength += (as.end-as.start);
            }
        }
        iter++;
    }
    if (BlinkMeasure::startTS < winStart) {
        watchedLength += (curTimestamp-winStart);
    } else {
        watchedLength += (curTimestamp-BlinkMeasure::startTS);
    }
    if (watchedLength < minWindowLength) {
        doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1: watchingWindowSize is too small %.2lf\n", watchedLength);
        return false;
    } else {
        doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1: watchingWindowSize %.2lf, checking whether to notify\n", watchedLength);
    }

    int blinksCount = 0;
    std::list<Blink>::iterator bIter;
    bIter = joinedBlinkChunksN1.begin();
    while(bIter != joinedBlinkChunksN1.end()) {
        Blink& b = *bIter;
        if (b.timestampStart < winStart) {
            bIter = joinedBlinkChunksN1.erase(bIter);
            continue;
        } else {
            bIter++;
            blinksCount++;
        }
    }
    double curRatio = ((double)blinksCount)/(watchedLength/1000.);
    doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1: min ratio:%.2f curRatio %.2f\n", minBlinksRatio, curRatio);

    if (curRatio < minBlinksRatio) {
        doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1: too few blinks %d\n", blinksCount);
        return true;
    } else {
        doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1: totalBlinks %d\n", blinksCount);
        return false;
    }
}

unsigned int BlinkMeasure::lastAddedToStateMachine = 0;
bool BlinkMeasure::delayStateMachine = true;

bool BlinkMeasure::n1UnderThreshold = false;
double BlinkMeasure::startTS = -1;
double BlinkMeasure::prevTS = -1;

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
                joinedBlinkChunksN1.push_back(joinedBlink);
                anyAdded = true;
                takenRBlinks.insert(rb.frameStart);
                doLog(debug_blinks_d5, "debug_blinks_d5: adding  fs %d fe %d start %.2lf end %lf duration %lf\n",
                    frameStart, frameEnd, timestampStart, timestampEnd, timestampEnd-timestampStart);
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
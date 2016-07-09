#include <list>
#include <stdio.h>

#include <opencv2/core/core.hpp>

#include <math.h>
#include <set>
#include <common.hpp>
#include <blinkmeasuref.hpp>

BlinkMeasureF::BlinkMeasureF() {
};
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

void BlinkMeasureF::measureBlinksSD(double *lSD, double *rSD, double *plsd1, double *prsd1, double *plsd2, double *prsd2, double *plsdt, double *prsdt, double *mlsd1, double *mrsd1, double *mlsd2, double *mrsd2, double *mlsdt, double *mrsdt) {
    int lSize, rSize;
    double lavg, ravg;
    std::list<BlinkMeasureF>::iterator iter;

    // mean for first SD
    lavg = 0; ravg = 0;
    lSize = 0; rSize = 0;
    iter = blinkMeasureShortf.begin();
    while(iter != blinkMeasureShortf.end()) {
        BlinkMeasureF& bm = *iter;
        if (bm.canProceedL == true) {
            lavg += bm.lDiffP.y;
            lSize++;
        }
        if (bm.canProceedR == true) {
            ravg += bm.rDiffP.y;
            rSize++;
        }
        iter++;
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
    iter = blinkMeasureShortf.begin();
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
    *plsd2 = lavg+(2*(*lSD));
    *prsd2 = ravg+(2*(*rSD));
    *mlsd2 = lavg-(2*(*lSD));
    *mrsd2 = ravg-(2*(*rSD));

    double doubleSD = true;
    if (doubleSD == true) {
        // second SD
        double plTmpSD2 = lavg+(2*(*lSD));
        double prTmpSD2 = ravg+(2*(*rSD));
        double mlTmpSD2 = lavg-(2*(*lSD));
        double mrTmpSD2 = ravg-(2*(*rSD));

        // mean for second SD
        lavg = 0; ravg = 0;
        lSize = 0; rSize = 0;
        iter = blinkMeasureShortf.begin();
        while(iter != blinkMeasureShortf.end()) {
            BlinkMeasureF& bm = *iter;
            if (bm.canProceedL == true) {
                if (mlTmpSD2 < bm.lDiffP.y && bm.lDiffP.y < plTmpSD2) {
                    lavg += bm.lDiffP.y;
                    lSize++;
                }
            }
            if (bm.canProceedR == true) {
                if (mrTmpSD2 < bm.rDiffP.y && bm.rDiffP.y < prTmpSD2) {
                    ravg += bm.rDiffP.y;
                    rSize++;
                }
            }
            iter++;
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
        *lSD = 0; *rSD = 0;
        lSize = 0; rSize = 0;
        iter = blinkMeasureShortf.begin();
        while(iter != blinkMeasureShortf.end()) {
            BlinkMeasureF& bm = *iter;
            if (bm.canProceedL == true) {
                if (mlTmpSD2 < bm.lDiffP.y && bm.lDiffP.y < plTmpSD2) {
                    *lSD = *lSD+pow(lavg-bm.lDiffP.y, 2);
                    lSize++;
                }
            }
            if (bm.canProceedR == true) {
                if (mrTmpSD2 < bm.rDiffP.y && bm.rDiffP.y < prTmpSD2) {
                    *rSD = *rSD+pow(ravg-bm.rDiffP.y, 2);
                    rSize++;
                }
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
    }
    *plsd1 = lavg+(1*(*lSD));
    *prsd1 = ravg+(1*(*rSD));
    *mlsd1 = lavg-(1*(*lSD));
    *mrsd1 = ravg-(1*(*rSD));
    *plsdt = lavg+(3.5*(*lSD));
    *prsdt = ravg+(3.5*(*rSD));
    *mlsdt = lavg-(4*(*lSD));
    *mrsdt = ravg-(4*(*rSD));
};
unsigned int BlinkMeasureF::lastAddedToStateMachine = 0;
bool BlinkMeasureF::delayStateMachine = true;
bool BlinkMeasureF::n1UnderThreshold = false;
double BlinkMeasureF::startTS = -1;
double BlinkMeasureF::prevTS = -1;
int    BlinkMeasureF::lCurState = 0;
double BlinkMeasureF::lLastVal = 0;
int    BlinkMeasureF::lZeroCrossPosToNegF = 0;
double BlinkMeasureF::lZeroCrossPosToNegT = 0;
int    BlinkMeasureF::rCurState = 0;
double BlinkMeasureF::rLastVal = 0;
int    BlinkMeasureF::rZeroCrossPosToNegF = 0;
double BlinkMeasureF::rZeroCrossPosToNegT = 0;

bool BlinkMeasureF::measureBlinks(BlinkMeasureF bm) {
    int timeWindow = 15;
    //int timeWindow = 10;

    // ensure that queue is long enough (at least 30 frames) (that we'll know how long we want to have it)
    int firstMeasureQueueSize = 30;
    int shortBmSize = blinkMeasureShortf.size();
    if (maxFramesShortList == 0) {
        if (shortBmSize < firstMeasureQueueSize) {
            blinkMeasureShortf.push_back(bm);
            return false;
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

    int minShortBmSize = maxFramesShortList/2;
    bool rewriteElementsToStateQueue = false;
    while (true && shortBmSize > 1) {
        BlinkMeasureF oldestBm = blinkMeasureShortf.front();
        if (oldestBm.timestamp > (bm.timestamp - (timeWindow*1000))) {
            break;
        } else {
            if (BlinkMeasureF::delayStateMachine == true) {
                if (oldestBm.frameNum > BlinkMeasureF::lastAddedToStateMachine
                    || (oldestBm.frameNum == 0 && BlinkMeasureF::lastAddedToStateMachine == 0)) {
                    // reliable shortsize length?
                    if (shortBmSize < minShortBmSize) {
                        blinkMeasureShortf.pop_front();
                        doLog(debug_blinks_d2, "debug_blinks_d2: F %d T %lf shortBmSize %d is less than minShortBmSize %d (skipping %d)\n",
                            bm.frameNum, bm.timestamp, shortBmSize, minShortBmSize, oldestBm.frameNum);
                    } else {
                        // reliable length, don't delete it, write it to state machine queue first!
                        rewriteElementsToStateQueue = true;
                        break;
                    }
                } else {
                    blinkMeasureShortf.pop_front();
                }
            } else {
                blinkMeasureShortf.pop_front();
            }
        }
        shortBmSize = blinkMeasureShortf.size();
    }

    if (BlinkMeasureF::delayStateMachine == false) {
        if (shortBmSize < minShortBmSize) {
            BlinkMeasureF::delayStateMachine = true;
        } else {
            BlinkMeasureF prevBm = blinkMeasureShortf.back();
            double lavg, ravg, lSD, rSD;
            double plsdt, prsdt, mlsdt, mrsdt, plsd1, prsd1, mlsd1, mrsd1, plsd2, prsd2, mlsd2, mrsd2;
            BlinkMeasureF::measureSD(&mlsdt, &plsdt, &mrsdt, &prsdt, &lavg, &ravg, &lSD, &rSD,
                &plsd1, &prsd1, &mlsd1, &mrsd1, &plsd2, &prsd2, &mlsd2, &mrsd2);
            BlinkMeasureF::processBm(prevBm, lavg, ravg, lSD, rSD, mlsdt, plsdt, mrsdt, prsdt, plsd1, prsd1, mlsd1, mrsd1, plsd2, prsd2, mlsd2, mrsd2);

            stateMachineElement sme;
            sme.bm = prevBm;
            sme.mlsdt = mlsdt; sme.plsdt = plsdt; sme.mrsdt = mrsdt; sme.prsdt = prsdt;
            doLog(debug_blinks_d2, "debug_blinks_d2: F %d T %lf pushing %u to stateMachineQueue\n", bm.frameNum, bm.timestamp, prevBm.frameNum);
            stateMachineQueue.push_back(sme);
            BlinkMeasureF::lastAddedToStateMachine = prevBm.frameNum;
        }
    }

    if (rewriteElementsToStateQueue == true) {
        // happens only if delayStateMachine is/was true
        BlinkMeasureF::delayStateMachine = false;
        BlinkMeasureF::rewriteElementsToStateQueue(bm.frameNum, bm.timestamp);
    }

    blinkMeasureShortf.push_back(bm);

    return (rewriteElementsToStateQueue == false && BlinkMeasureF::delayStateMachine == false);
}
void BlinkMeasureF::rewriteElementsToStateQueue(unsigned int frameNum, double timestamp) {
    double lavg, ravg, lSD, rSD;
    double plsdt, prsdt, mlsdt, mrsdt, plsd1, prsd1, mlsd1, mrsd1, plsd2, prsd2, mlsd2, mrsd2;
    BlinkMeasureF::measureSD(&mlsdt, &plsdt, &mrsdt, &prsdt, &lavg, &ravg, &lSD, &rSD,
        &plsd1, &prsd1, &mlsd1, &mrsd1, &plsd2, &prsd2, &mlsd2, &mrsd2);

    std::list<BlinkMeasureF>::iterator iter;
    iter = blinkMeasureShortf.begin();
    while(iter != blinkMeasureShortf.end()) {
        BlinkMeasureF& bmItem = *iter;
        BlinkMeasureF::processBm(bmItem, lavg, ravg, lSD, rSD, mlsdt, plsdt, mrsdt, prsdt, plsd1, prsd1, mlsd1, mrsd1, plsd2, prsd2, mlsd2, mrsd2);

        stateMachineElement sme;
        sme.bm = bmItem;
        sme.mlsdt = mlsdt; sme.plsdt = plsdt; sme.mrsdt = mrsdt; sme.prsdt = prsdt;
        doLog(debug_blinks_d2, "debug_blinks_d2: F %d T %lf pushing %u to stateMachineQueue\n", frameNum, timestamp, bmItem.frameNum);
        stateMachineQueue.push_back(sme);
        BlinkMeasureF::lastAddedToStateMachine = bmItem.frameNum;

        iter++;
    }
}

void BlinkMeasureF::processStateMachineQueue() {
    std::list<stateMachineElement>::iterator iter;
    iter = stateMachineQueue.begin();
    while(iter != stateMachineQueue.end()) {
        stateMachineElement& sme = *iter;
        BlinkMeasureF bm = sme.bm;

        //BlinkMeasureF::stateMachine(bm.frameNum, bm.timestamp, bm.lDiffP.y, mlsd2, plsd2, bm.rDiffP.y, mrsd2, prsd2);
        BlinkMeasureF::stateMachine(bm.frameNum, bm.timestamp, bm.lDiffP.y, sme.mlsdt, sme.plsdt, bm.rDiffP.y, sme.mrsdt, sme.prsdt);

        // track watching time for notifications
        if (BlinkMeasureF::prevTS == -1) {
            BlinkMeasureF::prevTS = bm.timestamp;
            BlinkMeasureF::startTS = bm.timestamp;
        } else {
            if ((bm.timestamp - BlinkMeasureF::prevTS) > 200) {
                activeSlice as;
                as.start = BlinkMeasureF::startTS;
                as.end = BlinkMeasureF::prevTS;
                n1ActiveSlices.push_back(as);
                BlinkMeasureF::startTS = bm.timestamp;
            }
            BlinkMeasureF::prevTS = bm.timestamp;
        }

        iter = stateMachineQueue.erase(iter);
    }
}

void BlinkMeasureF::processBm(BlinkMeasureF bm,
        double lavg, double ravg, double lSD, double rSD,
        double mlsdt, double plsdt, double mrsdt, double prsdt,
        double plsd1, double prsd1, double mlsd1, double mrsd1,
        double plsd2, double prsd2, double mlsd2, double mrsd2) {
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
}
void BlinkMeasureF::measureSD(double* mlsdt, double* plsdt, double* mrsdt, double* prsdt,
        double* lavg, double* ravg, double* lSD, double* rSD,
        double* plsd1, double* prsd1, double* mlsd1, double* mrsd1,
        double* plsd2, double* prsd2, double* mlsd2, double* mrsd2) {
    BlinkMeasureF::measureBlinksAVG(lavg, ravg);
    BlinkMeasureF::measureBlinksSD(lSD, rSD, plsd1, prsd1, plsd2, prsd2, plsdt, prsdt, mlsd1, mrsd1, mlsd2, mrsd2, mlsdt, mrsdt);
}

bool BlinkMeasureF::checkN1Notifs(double curTimestamp) {
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
    if (BlinkMeasureF::startTS < winStart) {
        watchedLength += (curTimestamp-winStart);
    } else {
        watchedLength += (curTimestamp-BlinkMeasureF::startTS);
    }
    if (watchedLength < minWindowLength) {
        doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1: watchingWindowSize is too small %.2lf\n", watchedLength);
        return false;
    } else {
        doLog(debug_notifications_n1_log1, "debug_notifications_n1_log1: watchingWindowSize %.2lf, checking whether to notify\n", watchedLength);
    }

    int blinksCount = 0;
    std::list<BlinkF>::iterator bIter;
    bIter = joinedBlinkChunksfN1.begin();
    while(bIter != joinedBlinkChunksfN1.end()) {
        BlinkF& b = *bIter;
        if (b.timestampStart < winStart) {
            bIter = joinedBlinkChunksfN1.erase(bIter);
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
void BlinkMeasureF::stateMachine(unsigned int frameNum, double timestamp, double leftY, double leftLowSD, double leftHighSD, double rightY, double rightLowSD, double rightHighSD) {
    if (BlinkMeasureF::lCurState == 0) {
        if (BlinkMeasureF::lLastVal == 0) {
            if (leftY == 0) {
                // do nothing
            } else {
                // first loop only
                BlinkMeasureF::lLastVal = leftY;
            }
        }
        if (BlinkMeasureF::lLastVal > 0 && leftY < 0) {
            BlinkMeasureF::lZeroCrossPosToNegF = frameNum;
            BlinkMeasureF::lZeroCrossPosToNegT = timestamp;
        }
        if (leftY < leftLowSD) {
            BlinkMeasureF::lAdding = true;
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
        if ((timestamp-BlinkMeasureF::lZeroCrossPosToNegT) > 800) {
            BlinkMeasureF::lCurState = 0;
        } else {
            if (BlinkMeasureF::lLastVal > 0 && leftY < 0) {
                BlinkF b(BlinkMeasureF::lZeroCrossPosToNegF, frameNum, BlinkMeasureF::lZeroCrossPosToNegT, timestamp, 0);
                lBlinkChunksf.push_back(b);
                doLog(debug_blinks_d4, "debug_blinks_d4: adding_lBlinkChunksf fs %d fe %d start %.2lf end %lf duration %lf\n",
                    BlinkMeasureF::lZeroCrossPosToNegF, frameNum, BlinkMeasureF::lZeroCrossPosToNegT, timestamp, timestamp-BlinkMeasureF::lZeroCrossPosToNegT);

                BlinkMeasureF::lZeroCrossPosToNegF = frameNum;
                BlinkMeasureF::lZeroCrossPosToNegT = timestamp;
                BlinkMeasureF::lAdding = false;
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
                // do nothing
            } else {
                // first loop only
                BlinkMeasureF::rLastVal = rightY;
            }
        }
        if (BlinkMeasureF::rLastVal > 0 && rightY < 0) {
            BlinkMeasureF::rZeroCrossPosToNegF = frameNum;
            BlinkMeasureF::rZeroCrossPosToNegT = timestamp;
        }
        if (rightY < rightLowSD) {
            BlinkMeasureF::rAdding = true;
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
        if ((timestamp-BlinkMeasureF::rZeroCrossPosToNegT) > 800) {
            BlinkMeasureF::rCurState = 0;
        } else {
            if (BlinkMeasureF::rLastVal > 0 && rightY < 0) {
                BlinkF b(BlinkMeasureF::rZeroCrossPosToNegF, frameNum, BlinkMeasureF::rZeroCrossPosToNegT, timestamp, 0);
                rBlinkChunksf.push_back(b);
                doLog(debug_blinks_d4, "debug_blinks_d4: adding_rBlinkChunksf fs %d fe %d start %.2lf end %lf duration %lf\n",
                    BlinkMeasureF::rZeroCrossPosToNegF, frameNum, BlinkMeasureF::rZeroCrossPosToNegT, timestamp, timestamp-BlinkMeasureF::rZeroCrossPosToNegT);

                BlinkMeasureF::rZeroCrossPosToNegF = frameNum;
                BlinkMeasureF::rZeroCrossPosToNegT = timestamp;
                BlinkMeasureF::rAdding = false;
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

bool BlinkMeasureF::joinBlinks() {
    bool anyAdded = false;
    if (BlinkMeasureF::lAdding == true || BlinkMeasureF::rAdding == true) {
        // waiting that blinks from both eyes are finished
        return false;
    }
    std::set<unsigned int> takenRBlinks;

    std::list<BlinkF>::iterator lIter = lBlinkChunksf.begin();
    while(lIter != lBlinkChunksf.end()) {
        BlinkF& lb = *lIter;

        std::list<BlinkF>::iterator rIter = rBlinkChunksf.begin();
        while(rIter != rBlinkChunksf.end()) {
            BlinkF& rb = *rIter;
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
                BlinkF joinedBlink(frameStart, frameEnd, timestampStart, timestampEnd, 0);
                joinedBlinkChunksfN1.push_back(joinedBlink);
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

    lBlinkChunksf.clear();
    rBlinkChunksf.clear();
    return anyAdded;
}

void BlinkMeasureF::makeNotification(bool isLeft) {
    if (isLeft == true) {
    } else {
        // right
    }
};

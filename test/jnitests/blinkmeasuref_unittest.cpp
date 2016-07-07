#include "opencv2/imgproc/imgproc.hpp"

#include "gtest/gtest.h"

#include <common.hpp>
#include <blinkmeasuref.hpp>

TEST(JoinBlinksMissingLR, Regular) {
    joinedBlinkChunksf.clear();
    lBlinkChunksf.clear();
    rBlinkChunksf.clear();

    lBlinkChunksf.push_back(BlinkF(1, 5, 100, 200, 0));

    EXPECT_EQ(false, BlinkMeasureF::joinBlinks());
    EXPECT_EQ(0, lBlinkChunksf.size());
    EXPECT_EQ(0, rBlinkChunksf.size());

    EXPECT_EQ(0, joinedBlinkChunksf.size());

    joinedBlinkChunksf.clear();
    lBlinkChunksf.clear();
    rBlinkChunksf.clear();

    rBlinkChunksf.push_back(BlinkF(1, 5, 100, 200, 0));

    EXPECT_EQ(false, BlinkMeasureF::joinBlinks());
    EXPECT_EQ(0, lBlinkChunksf.size());
    EXPECT_EQ(0, rBlinkChunksf.size());

    EXPECT_EQ(0, joinedBlinkChunksf.size());
}

TEST(JoinBlinksNotAligned, Regular) {
    joinedBlinkChunksf.clear();
    lBlinkChunksf.clear();
    rBlinkChunksf.clear();

    lBlinkChunksf.push_back(BlinkF(1, 5, 100, 200, 0));
    rBlinkChunksf.push_back(BlinkF(6, 8, 100, 200, 0));
    lBlinkChunksf.push_back(BlinkF(9, 11, 100, 200, 0));
    rBlinkChunksf.push_back(BlinkF(12, 15, 100, 200, 0));

    EXPECT_EQ(false, BlinkMeasureF::joinBlinks());
    EXPECT_EQ(0, lBlinkChunksf.size());
    EXPECT_EQ(0, rBlinkChunksf.size());

    EXPECT_EQ(0, joinedBlinkChunksf.size());

    joinedBlinkChunksf.clear();
    lBlinkChunksf.clear();
    rBlinkChunksf.clear();
}

TEST(JoinBlinksMulti, Regular) {
    joinedBlinkChunksf.clear();
    lBlinkChunksf.clear();
    rBlinkChunksf.clear();

    int expectedFrames[4][2] = {
        {50, 110},
        {105, 200},
        {198, 400},
        {270, 500},
    };
    lBlinkChunksf.push_back(BlinkF(50, 100, 100, 200, 0));
    rBlinkChunksf.push_back(BlinkF(70, 110, 100, 200, 0));

    lBlinkChunksf.push_back(BlinkF(105, 200, 100, 200, 0));
    rBlinkChunksf.push_back(BlinkF(120, 125, 100, 200, 0));

    rBlinkChunksf.push_back(BlinkF(126, 130, 100, 200, 0)); // knocked out
    rBlinkChunksf.push_back(BlinkF(135, 140, 100, 200, 0)); // knocked out

    rBlinkChunksf.push_back(BlinkF(198, 400, 100, 200, 0));
    lBlinkChunksf.push_back(BlinkF(230, 240, 100, 200, 0));

    lBlinkChunksf.push_back(BlinkF(245, 250, 100, 200, 0)); // knocked out
    lBlinkChunksf.push_back(BlinkF(251, 260, 100, 200, 0)); // knocked out

    lBlinkChunksf.push_back(BlinkF(270, 500, 100, 200, 0));
    rBlinkChunksf.push_back(BlinkF(430, 500, 100, 200, 0));
    EXPECT_EQ(true, BlinkMeasureF::joinBlinks());
    EXPECT_EQ(0, lBlinkChunksf.size());
    EXPECT_EQ(0, rBlinkChunksf.size());

    EXPECT_EQ(4, joinedBlinkChunksf.size());
    std::list<BlinkF>::iterator jIter = joinedBlinkChunksf.begin();
    int i=0;
    while(jIter != joinedBlinkChunksf.end()) {
        BlinkF& jb = *jIter;
        //printf("%u-%u\n", jb.frameStart, jb.frameEnd);
        EXPECT_EQ(expectedFrames[i][0], jb.frameStart);
        EXPECT_EQ(expectedFrames[i][1], jb.frameEnd);
        jIter++;
        i++;
    }
}



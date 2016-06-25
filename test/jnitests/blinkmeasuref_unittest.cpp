#include "opencv2/imgproc/imgproc.hpp"

#include "gtest/gtest.h"

#include <blinkmeasuref.hpp>

TEST(StateMachineTest, Regular) {
    BlinkMeasureF::lLastVal = 5;
    //BlinkMeasureF::stateMachine(

    EXPECT_EQ(true, 5 ==5);
}

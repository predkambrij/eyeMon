#include "opencv2/imgproc/imgproc.hpp"

#include "gtest/gtest.h"

#include <templatebased.hpp>

TEST(UpdateSearchRegionTest, Regular) {
    TemplateBased templ;
    templ.lEye = cv::Point(10, 11);
    templ.rEye = cv::Point(12, 13);
    templ.lLastTime = 100;
    templ.rLastTime = 100;
    templ.hasTemplate = true;

    templ.checkTracking(150);
    EXPECT_EQ(true, templ.hasTemplate);

    templ.checkTracking(1500);
    EXPECT_EQ(false, templ.hasTemplate);
}

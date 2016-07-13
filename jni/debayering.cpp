#include <dirent.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


cv::Mat convert(const char* fname) {
    chdir("/home/developer/other/basler/poli1person6_/");
    cv::Mat frame = cv::imread(fname, cv::IMREAD_GRAYSCALE);
    int width = 640, height = 480;
    cv::Mat rgb8BitMat(height, width, CV_8UC3);
    cv::cvtColor(frame, rgb8BitMat, CV_BayerGR2BGR);
    return rgb8BitMat;
}

int main(void) {
    struct dirent **namelist;
    int i=0, n;

    // codec
    //int ex = CV_FOURCC('L','A','G','S'); // doesn't work
    //int ex = CV_FOURCC('X', 'V', 'I', 'D');
    int ex = CV_FOURCC('F', 'M', 'P', '4'); // H. 263
    //int ex = CV_FOURCC('X', '2', '6', '4');
    //int ex = 0; // uncompressed
    //int ex = -1;
    cv::Size s = cv::Size(640, 480);
    cv::VideoWriter outputVideo;
    outputVideo.open("/home/developer/other/basler/poli1person6/poli1person6_x263.avi", ex, 100, s, true);
    if (!outputVideo.isOpened()) {
        std::cout  << "Could not open the output video for write: " << std::endl;
        return -1;
    }
//return 0;
    n = scandir("/home/developer/other/basler/poli1person6_/", &namelist, NULL, alphasort);
    while (i < n) {
        //std::string tmps(namelist[i]->d_name);
        if (strncmp(namelist[i]->d_name+strlen(namelist[i]->d_name)-4 , ".png" , 4) != 0) {
            i++;
            continue;
        }
        printf("%s\n", namelist[i]->d_name);
        cv::Mat clr = convert(namelist[i]->d_name);
        //outputVideo.write(clr);
        outputVideo << clr;

        free(namelist[i]);
        i++;
    }
    free(namelist);
}
/*
int main(void) {
    chdir("/home/developer/other/basler/poli1person10_/");
    cv::Mat frame = cv::imread("12-03-31.354.png", cv::IMREAD_GRAYSCALE);
    int width = 640, height = 480;
    cv::Mat rgb8BitMat(height, width, CV_8UC3);
    cv::cvtColor(frame, rgb8BitMat,  CV_BayerGR2BGR);
    cv::namedWindow("Display window", CV_WINDOW_AUTOSIZE);
    cv::imshow("Display window", rgb8BitMat);
    cv::waitKey(0);
    return 0;
}
*/
/*
possible codes http://docs.opencv.org/3.1.0/df/d4e/group__imgproc__c.html#gsc.tab=0
int main() {
    DIR *dir;
    struct dirent *dp;
    dir = opendir("/home/developer/other/basler/poli1person2/");
    while ((dp = readdir(dir)) != NULL) {
        printf ("%s\n", dp->d_name);
    }
    closedir(dir);
}
*/
/*
process
    // Copy the data into an OpenCV Mat structure
    //cv::Mat bayer16BitMat(height, width, CV_16UC1, inputBuffer);

    // Convert the Bayer data from 16-bit to to 8-bit
    //cv::Mat bayer8BitMat = bayer16BitMat.clone();
    // The 3rd parameter here scales the data by 1/16 so that it fits in 8 bits.
    // Without it, convertTo() just seems to chop off the high order bits.
    //bayer8BitMat.convertTo(bayer8BitMat, CV_8UC1, 0.0625);

    // Convert the Bayer data to 8-bit RGB
    //imwrite("/opt/docker_volumes/mag/home_developer/other/basler/poli1person2out/test.png", toSave);

    cv::namedWindow("Display window", CV_WINDOW_AUTOSIZE);
    cv::imshow("Display window", rgb8BitMat);
    cv::waitKey(0);
*/


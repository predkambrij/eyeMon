
#include <optflow.cpp>


int main() {
    PHONE = 0; farne = 0;
    // test videos
    // char fileName[100] = "/home/developer/other/posnetki/o4_29.mp4";
    // char fileName[100] = "/home/developer/other/posnetki/o4_30.mp4";
    char fileName[100] = "/home/developer/other/posnetki/o4_31.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/crnc1.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/indian_close.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/yellow_close.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/very_dark.mp4";
    // char fileName[100] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
    VideoCapture stream1(fileName);   //0 is the id of video device.0 if you have only one camera
    // VideoCapture stream1(0);
    if (!stream1.isOpened()) {
        CV_Assert("Cam open failed");
    }

    // resolutions 320, 240; 800, 448; 640, 480
    stream1.set(CV_CAP_PROP_FRAME_WIDTH, 640); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

    char faceDetector[200] = "/home/developer/other/android_deps/OpenCV-2.4.10-android-sdk/samples/optical-flow/res/raw/lbpcascade_frontalface.xml";
    // printf("video capture %f %f %f\n", stream1.get(CV_CAP_PROP_FRAME_WIDTH), stream1.get(CV_CAP_PROP_FRAME_HEIGHT), stream1.get(CV_CAP_PROP_FPS));

    // controls
    int pause = 0, firstLoop = 1;
    clock_t loopStart, start;
    Mat frame, gray, cflow;

    setUp(faceDetector);

    if (farne == 0) {
        cv::namedWindow(face_window_name,CV_WINDOW_NORMAL); cv::moveWindow(face_window_name, 10, 100);
        cv::namedWindow("main",CV_WINDOW_NORMAL); cv::moveWindow("main", 10, 100); resizeWindow("main",1280, 960);
        // cv::namedWindow("Right Eye",CV_WINDOW_NORMAL); cv::moveWindow("Right Eye", 10, 600);
        // cv::namedWindow("Left Eye",CV_WINDOW_NORMAL); cv::moveWindow("Left Eye", 10, 800);
        // createCornerKernels(), at the end // releaseCornerKernels(); // ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2), 43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);
    }

    while (true) {
        if(!(stream1.read(frame))) {
            doLog(" --(!) No captured frame -- Break!");
            return 0;
        }
        // cv::flip(frame, frame, 1);

        if (firstLoop == 1) {
            loopStart = clock();
            firstLoop = 0;
            continue;
        }

        diffclock("\nwhole loop", loopStart);
        loopStart = clock();

        start = clock();
        getGray(frame, &gray);
        diffclock("getGray", start);

        start = clock();
        process(frame, gray, frame);
        diffclock("process", start);

        // flow control
        int c = cv::waitKey(10);
        if(c == 27) { // esc
            break;
        } else if((char)c == 'p') {
            pause = 1;
        } else if((char)c == 'f') {
            flg = 1;
            // imwrite("/tmp/frame.png",cflow);
        } else if (pause == 1) {
            while (true) {
                int c = cv::waitKey(10);
                if((char)c == 'p') {
                    pause = 0;
                    break;
                } else if((char)c == 'n') {
                    break;
                }
            }
        }
    }

    return 0;
}

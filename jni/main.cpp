
#include <list>
#include <thread>


#include <optflow.cpp>
#include <templatebased.cpp>

class FrameCarrier {
    public: Mat frame;
    public: double timestamp;

    public: FrameCarrier(Mat frame, double timestamp) {
        this->frame     = frame;
        this->timestamp = timestamp;

    }
};

std::list<FrameCarrier> frameList;
int maxSize = 3;
bool canAdd = true;

void captureFrames() {
    // test videos
    // char fileName[100] = "/home/developer/other/posnetki/o4_29.mp4";
    // char fileName[100] = "/home/developer/other/posnetki/o4_30.mp4";
    //char fileName[100] = "/home/developer/other/posnetki/o4_31.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/crnc1.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/indian_close.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/yellow_close.mp4";
    // char fileName[200] = "/home/developer/other/test_videos/very_dark.mp4";
    // char fileName[100] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
    char fileName[100] = "/home/developer/other/posnetki/o4_44.mp4";
    //VideoCapture stream1(fileName);   //0 is the id of video device.0 if you have only one camera
    bool isVideoCapture = false;
    if (isVideoCapture) {
        maxSize = 3000000000;
    }

    VideoCapture stream1(0);
    if (!stream1.isOpened()) {
        CV_Assert("T1 cam open failed");
    }
    // resolutions 320, 240; 800, 448; 640, 480
    //stream1.set(CV_CAP_PROP_FRAME_WIDTH, 320); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
    stream1.set(CV_CAP_PROP_FRAME_WIDTH, 640); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    //stream1.set(CV_CAP_PROP_FRAME_WIDTH, 1280); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

    if (debug_t1_log == true) {
        printf("T1 video capture %f %f %f\n", stream1.get(CV_CAP_PROP_FRAME_WIDTH), stream1.get(CV_CAP_PROP_FRAME_HEIGHT), stream1.get(CV_CAP_PROP_FPS));
        printf("CAP_PROP_FPS %f\n", stream1.get(CV_CAP_PROP_FPS));
        printf("CAP_PROP_FRAME_COUNT %f\n", stream1.get(CV_CAP_PROP_FRAME_COUNT));
    }

    Mat frame;
    std::chrono::time_point<std::chrono::steady_clock> t1 = std::chrono::steady_clock::now();
    while (true) {
        if(!(stream1.read(frame))) {
            if (debug_t1_log == true) {
                doLog("T1 --(!) No captured frame -- Break!");
            }
            return;
        }

        if (debug_t1_log == true) {
            difftime("T1 frame capture:", t1);
            t1 = std::chrono::steady_clock::now();
        }

        long unsigned int listSize = frameList.size();
        if (debug_t1_log == true) {
            printf("size %ld\n", frameList.size());
        }
        if (listSize >= maxSize) {
            if (debug_t1_log == true) {
                printf("T1 reached max size %d\n", maxSize);
            }
            canAdd = false;
        } else {
            if (canAdd == true) {
                double frameTimeMs;
                if (isVideoCapture == true) {
                    frameTimeMs = (double) stream1.get(CV_CAP_PROP_POS_MSEC);
                } else {
                    long int ft = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
                    frameTimeMs = (double) ft;
                }
                FrameCarrier fc(frame.clone(), frameTimeMs);
                frameList.push_back(fc);
            }
        }
    }
}

OptFlow optf;
TemplateBased templ;

void doProcessing() {
    //cv::namedWindow(face_window_name,CV_WINDOW_NORMAL); cv::moveWindow(face_window_name, 10, 100);
    if (debug_show_img_main == true) {
        cv::namedWindow("main",CV_WINDOW_NORMAL); cv::moveWindow("main", 400, 100); resizeWindow("main",1280, 960);
    }
    if (debug_show_img_face == true) {
        cv::namedWindow("face",CV_WINDOW_NORMAL); cv::moveWindow("face", 400, 100);
    }
    if (debug_show_img_templ_eyes_tmpl == true) {
        cv::namedWindow("left",CV_WINDOW_NORMAL); cv::moveWindow("left", 1300, 500);
        cv::namedWindow("right",CV_WINDOW_NORMAL); cv::moveWindow("right", 1600, 500);
    }
    if (debug_show_img_templ_eyes_cor == true) {
        cv::namedWindow("leftR",CV_WINDOW_NORMAL); cv::moveWindow("leftR", 1300, 800);
        cv::namedWindow("rightR",CV_WINDOW_NORMAL); cv::moveWindow("rightR", 1600, 800);
    }
    /*
    cv::namedWindow("leftR1",CV_WINDOW_NORMAL); cv::moveWindow("leftR1", 10, 800);
    cv::namedWindow("rightR1",CV_WINDOW_NORMAL); cv::moveWindow("rightR1", 200, 800);
    */
    // cv::namedWindow("Right Eye",CV_WINDOW_NORMAL); cv::moveWindow("Right Eye", 10, 600);
    // cv::namedWindow("Left Eye",CV_WINDOW_NORMAL); cv::moveWindow("Left Eye", 10, 800);
    // createCornerKernels(), at the end // releaseCornerKernels(); // ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2), 43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);

    std::chrono::time_point<std::chrono::steady_clock> t1 = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> t2;
    Mat frame, gray, cflow;

    while (true) {
        long unsigned int listSize = frameList.size();
        if (listSize == 0) {
            if (canAdd == false) {
                canAdd = true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        FrameCarrier fc = frameList.front();
        frameList.pop_front();
        Mat frame = fc.frame;
        // cv::flip(frame, frame, 1);
        double timestamp = fc.timestamp;
        if (debug_t2_log == true) {
            printf("T2 frame time:%lf\n", timestamp);
        }

        t2 = std::chrono::steady_clock::now();
        switch (method) {
            case METHOD_OPTFLOW:
            case METHOD_TEMPLATE_BASED:
            getGray(frame, &gray);
            break;
            case METHOD_BLACK_PIXELS:
            break;
        }
        if (debug_t2_log == true) {
            difftime("T2 getGray", t2);
        }

        t2 = std::chrono::steady_clock::now();
        switch (method) {
            case METHOD_OPTFLOW:
            optf.run(gray, frame);
            break;
            case METHOD_TEMPLATE_BASED:
            templ.run(gray, frame, timestamp);
            break;
            case METHOD_BLACK_PIXELS:
            break;
        }
        if (debug_t2_log == true) {
            difftime("T2 run", t2);
        }

        if (debug_show_img_main == true) {
            // flow control
            int c = cv::waitKey(10);
            if((char)c == 'q') {
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
        if (debug_t2_log == true) {
            difftime("T2 whole loop:", t1);
        }
        t1 = std::chrono::steady_clock::now();
    }
}

int main() {
    PHONE = 0; farne = 0;

    char faceDetector[200] = "/home/developer/other/android_deps/OpenCV-2.4.10-android-sdk/samples/optical-flow/res/raw/lbpcascade_frontalface.xml";
    switch (method) {
        case METHOD_OPTFLOW:
        optf.setup(faceDetector);
        break;
        case METHOD_TEMPLATE_BASED:
        templ.setup(faceDetector);
        break;
        case METHOD_BLACK_PIXELS:
        break;
    }

    thread t1(captureFrames);
    thread t2(doProcessing);

    t1.join();
    t2.join();

    return 0;
}

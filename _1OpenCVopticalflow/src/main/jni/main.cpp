
#include <list>
#include <thread>
#include <fstream>

#include <common.hpp>

#include <optflow.hpp>
#include <templatebased.hpp>
#include <farneback.hpp>
#include <blackpixels.hpp>

#include <main_settings.hpp>

void getGray(cv::Mat frame, cv::Mat *gray) {
    // int resizeFactor = 1;
    //resize(frame, *output, Size(frame.size().width/resizeFactor, frame.size().height/resizeFactor));
    //cvtColor(frame, *gray, CV_BGR2GRAY);
    std::vector<cv::Mat> rgbChannels(3);
    cv::split(frame, rgbChannels);
    *gray = rgbChannels[2];
}

class FrameCarrier {
    public: cv::Mat frame;
    public: unsigned int frameNum;
    public: double timestamp;

    public: FrameCarrier(cv::Mat frame, double timestamp, unsigned int frameNum) {
        this->frame     = frame;
        this->timestamp = timestamp;
        this->frameNum  = frameNum;
    }
};

std::list<FrameCarrier> frameList;
bool canAdd = true;
bool finished = false;
bool grabbing = true;

void captureFrames() {
    if (!stream1.isOpened()) {
        CV_Assert("T1 cam open failed");
    }

    if (isVideoCapture == true) {
        // resolutions 320, 240; 800, 448; 640, 480
        //stream1.set(CV_CAP_PROP_FRAME_WIDTH, 320); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
        stream1.set(CV_CAP_PROP_FRAME_WIDTH, 640); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        //stream1.set(CV_CAP_PROP_FRAME_WIDTH, 1280); stream1.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
    }

    if (isVideoCapture == false) {
        doLog(debug_t1_log, "debug_t1_log: T1 video capture %f %f %f\n", stream1.get(CV_CAP_PROP_FRAME_WIDTH), stream1.get(CV_CAP_PROP_FRAME_HEIGHT), stream1.get(CV_CAP_PROP_FPS));
        doLog(debug_t1_log, "debug_t1_log: CAP_PROP_FPS %f\n", stream1.get(CV_CAP_PROP_FPS));
        doLog(debug_t1_log, "debug_t1_log: CAP_PROP_FRAME_COUNT %f\n", stream1.get(CV_CAP_PROP_FRAME_COUNT));
    }

    cv::Mat frame;
    unsigned int frameNum = 0;
    //std::chrono::time_point<std::chrono::steady_clock> t1 = std::chrono::steady_clock::now();
    double prevFrameMs = 0;
    while (grabbing) {
#ifdef IS_TEST
//        while (canAdd != true && grabbing) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(10));
//        }
#endif
        if(!(stream1.read(frame))) {
            doLog(debug_t1_log, "debug_t1_log: No captured frame, exiting!\n");
            finished = true;
            return;
        }

        long int listSize = frameList.size();
        doLog(debug_t1_log, "debug_t1_log: size %ld\n", frameList.size());

        if (listSize >= maxSize && disable_max_size != true) {
            doLog(debug_t1_log, "debug_t1_log: reached max size %d\n", maxSize);
            canAdd = false;
        }
        while (canAdd == false && grabbing) {
            doLog(debug_t1_log, "debug_t1_log: waiting...\n", frameList.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        canAdd = true;
        double frameTimeMs;
        if (isVideoCapture == true) {
            long int ft = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())).count();
            frameTimeMs = (double) ft;
        } else {
            if (shouldUseAnnotTimestamps == true) {
                if (annotTimestampsMap.find((int)frameNum)!=annotTimestampsMap.end()) {
                    frameTimeMs = annotTimestampsMap[(int)frameNum];
                } else {
                    printf("missing framenum %d\n", (int)frameNum);
                    return;
                }
                
            } else {
                frameTimeMs = (double) stream1.get(CV_CAP_PROP_POS_MSEC);
            }
        }
        if (frameNum >= startingFrameNum) {
            FrameCarrier fc(frame.clone(), frameTimeMs, frameNum);
            frameList.push_back(fc);
            doLog(debug_t1_log, "debug_t1_log: F %d T %lf diff %lf\n", frameNum, frameTimeMs, frameTimeMs-prevFrameMs);
            prevFrameMs = frameTimeMs;
        }
        if (frameNum == endingFrameNum && endingFrameNum != 0) {
            doLog(debug_t1_log, "debug_t1_log: F %d T %lf  finished by endingFrameNum\n", frameNum, frameTimeMs);
            finished = true;
            return;
        }
        frameNum++;
    }
}

OptFlow optf;
Farneback farneback;
Blackpixels blackpixels;
TemplateBased templ;

void doProcessing() {
    //cv::namedWindow(face_window_name,CV_WINDOW_NORMAL); cv::moveWindow(face_window_name, 10, 100);
    if (debug_show_img_d1 == true) {
        cv::namedWindow("debug1",CV_WINDOW_NORMAL); cv::moveWindow("debug1", 60, 220);
        cv::namedWindow("debug2",CV_WINDOW_NORMAL); cv::moveWindow("debug2", 60, 490);
        cv::namedWindow("debug3",CV_WINDOW_NORMAL); cv::moveWindow("debug3", 60, 790);
        cv::namedWindow("debug4",CV_WINDOW_NORMAL); cv::moveWindow("debug4", 60, 30);
    }
    if (debug_show_img_main == true) {
        cv::namedWindow("main",CV_WINDOW_NORMAL); cv::moveWindow("main", 400, 30); cv::resizeWindow("main",1280, 960);
    }
    if (debug_show_img_gray == true) {
        cv::namedWindow("gray",CV_WINDOW_NORMAL); cv::moveWindow("gray", 400, 100); cv::resizeWindow("gray",1280, 960);
    }
    if (debug_show_img_face == true) {
        cv::namedWindow("face",CV_WINDOW_NORMAL); cv::moveWindow("face", 60, 30);
    }
    if (debug_show_img_farne_eyes == true && debug_show_img_main == true && (method == METHOD_FARNEBACK || method == METHOD_BLACKPIXELS)) {
        cv::namedWindow("leftR",CV_WINDOW_NORMAL); cv::moveWindow("leftR", 1300, 800);
        cv::namedWindow("rightR",CV_WINDOW_NORMAL); cv::moveWindow("rightR", 1600, 800);
        cv::namedWindow("left",CV_WINDOW_NORMAL); cv::moveWindow("left", 1300, 500);
        cv::namedWindow("right",CV_WINDOW_NORMAL); cv::moveWindow("right", 1600, 500);
        cv::namedWindow("leftSR",CV_WINDOW_NORMAL); cv::moveWindow("leftSR", 1300, 200);
        cv::namedWindow("rightSR",CV_WINDOW_NORMAL); cv::moveWindow("rightSR", 1600, 200);
    }
    if (debug_show_img_templ_eyes_tmpl == true && method == METHOD_TEMPLATE_BASED) {
        cv::namedWindow("leftSR",CV_WINDOW_NORMAL); cv::moveWindow("leftSR", 1300, 200);
        cv::namedWindow("rightSR",CV_WINDOW_NORMAL); cv::moveWindow("rightSR", 1600, 200);
    }
    if (debug_show_img_templ_eyes_tmpl == true && method == METHOD_TEMPLATE_BASED) {
        cv::namedWindow("left",CV_WINDOW_NORMAL); cv::moveWindow("left", 1300, 500);
        cv::namedWindow("right",CV_WINDOW_NORMAL); cv::moveWindow("right", 1600, 500);
    }
    if (debug_show_img_templ_eyes_cor == true && method == METHOD_TEMPLATE_BASED) {
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
    cv::Mat frame, gray, cflow;
    unsigned int lastFrameNum;
    double lastTimestamp;
    while (true) {
        long unsigned int listSize = frameList.size();
        if (listSize == 0) {
            if (finished == true) {
                break;
            }
            if (canAdd == false) {
                canAdd = true;
            }
            t2 = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            difftime("debug_t2_perf: waiting for frames", t2, debug_t2_perf);
            continue;
        }

        FrameCarrier fc = frameList.front();
        lastFrameNum = fc.frameNum;
        lastTimestamp = fc.timestamp;
        frameList.pop_front();
        cv::Mat frame = fc.frame;
        // cv::flip(frame, frame, 1);
        double timestamp = fc.timestamp;
        doLog(debug_t2_log, "debug_t2_log: frame time: %lf\n", timestamp);

        t2 = std::chrono::steady_clock::now();
        switch (method) {
            case METHOD_FARNEBACK:
            case METHOD_BLACKPIXELS:
            case METHOD_OPTFLOW:
            case METHOD_TEMPLATE_BASED:
            getGray(frame, &gray);
            break;
            case METHOD_BLACK_PIXELS:
            break;
        }
        difftime("debug_t2_perf: getGray", t2, debug_t2_perf);

        t2 = std::chrono::steady_clock::now();
        switch (method) {
            case METHOD_OPTFLOW:
            optf.run(gray, frame, timestamp, fc.frameNum);
            break;
            case METHOD_FARNEBACK:
            farneback.run(gray, frame, timestamp, fc.frameNum);
            break;
            case METHOD_BLACKPIXELS:
            blackpixels.run(gray, frame, timestamp, fc.frameNum);
            break;
            case METHOD_TEMPLATE_BASED:
            templ.run(gray, frame, timestamp, fc.frameNum);
            break;
            case METHOD_BLACK_PIXELS:
            break;
        }
        difftime("debug_t2_perf_method:", t2, debug_t2_perf_method);


        t2 = std::chrono::steady_clock::now();
        if (debug_show_img_main == true) {
            // flow control
            int c = cv::waitKey(1);
            if((char)c == 'q') {
                grabbing = false;
                break;
            } else if((char)c == 'p') {
                pauseFrames = 1;
            } else if((char)c == 'f') {
                flg = 1;
            } else if (pauseFrames == 1) {
                while (true) {
                    int c = cv::waitKey(10);
                    if((char)c == 'p') {
                        pauseFrames = 0;
                        break;
                    } else if((char)c == 'i') {
                        imwrite("/tmp/frame.png", toSave);
                    } else if((char)c == 'n') {
                        break;
                    } else if((char)c == 's') {
                        // status
                        printStatus();
                        break;
                    }
                }
            }
        }
        difftime("debug_t2_perf: waitkey", t2, debug_t2_perf);
        difftime("debug_t2_perf_whole:", t1, debug_t2_perf_whole);
        t1 = std::chrono::steady_clock::now();
    }
    // end hook
    switch (method) {
        case METHOD_OPTFLOW:
        break;
        case METHOD_FARNEBACK:
        doLog(debug_fb_log_tracking, "debug_fb_log_tracking: F %u T %.3lf status stop\n", lastFrameNum, lastTimestamp);
        farneback.flushMeasureBlinks();
        break;
        case METHOD_BLACKPIXELS:
        break;
        case METHOD_TEMPLATE_BASED:
        templ.flushMeasureBlinks();
        break;
        case METHOD_BLACK_PIXELS:
        break;
    }
    doLog(true, "exiting\n");
}
///
char* getCmdOption(char** begin, char** end, const std::string& option) {
    char** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }

    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}
///
void processOptions(int argc, char* argv[]) {
    /*
    if(cmdOptionExists(argv, argv+argc, "-h")) {
        // Do stuff
    }
    */
    char* filename = getCmdOption(argv, argv + argc, "--debug_show_img_main");
    if (filename) {
        // Do interesting things
    }
}
bool getTagName(std::string& str) {
    std::string from = ".avi";
    std::string to = ".tag";
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
bool getTimestampsName(std::string& str) {
    std::string from = ".avi";
    std::string to = ".txt";
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
int main() { // int argc, char * argv[]
    //processOptions(argc, argv);
    //return 0;
#ifdef IS_TEST
    if (shouldUseAnnotEyePosition == true || debug_fb_log_pupil_coverage == true) {
        std::string tagNameS(fileName);
        bool res = getTagName(tagNameS);
        if (res == false) {
            printf("Cannot provide tag name\n");
            return 1;
        }
        std::ifstream f;
        f.open(tagNameS.c_str(), std::fstream::in);
        if (!f.good()) {
            printf("It seems that annotations doesn't exist\n");
            return 2;
        }
        // skip the header
        while(true) {
            char line[1000];
            f.getline(line, 1000);
            if (strncmp(line , "#start" , 6) == 0) {
                break;
            }
        }
        // parse annotations
        while(true) {
            char line[1000];
            f.getline(line, 1000);
            if (strncmp(line , "#end", 4) == 0) {
                break;
            }
            annotEyePosition annot;
            long int frameCounter;
            int blinkID;
            char nonFrontalFace, leftFullyClosed, leftNonFrontal, rightFullyClosed, rightNonFrontal;
            int faceX, faceY, faceWidth, faceHeight;
            sscanf(line, "%ld:%d:%c:%c:%c:%c:%c:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
                &frameCounter, &blinkID, &nonFrontalFace,&leftFullyClosed,&leftNonFrontal,&rightFullyClosed,&rightNonFrontal,
                &faceX, &faceY, &faceWidth, &faceHeight,
                &annot.l1x, &annot.l1y, &annot.l2x, &annot.l2y,
                &annot.r1x, &annot.r1y, &annot.r2x, &annot.r2y);
            annotEyePositionMap[frameCounter] = annot;
        }

        //printf("seems we did it %lu\n", annotEyePositionMap.size());
        //printf("%d %d (%d %d)\n", annotEyePositionMap[1].lx1, annotEyePositionMap[1].ly1, , annotEyePositionMap.find(1)!=annotEyePositionMap.end());
    }
    if (shouldUseAnnotTimestamps == true) {
        std::string tsNameS(fileName);
        bool res = getTimestampsName(tsNameS);
        if (res == false) {
            printf("Cannot provide timestamps name\n");
            return 1;
        }
        std::ifstream f;
        f.open(tsNameS.c_str(), std::fstream::in);
        if (!f.good()) {
            printf("It seems that timestamps doesn't exist\n");
            return 2;
        }
        int frameCounter;
        while(f >> frameCounter) {
            double timestamp;
            f>>timestamp;
            annotTimestampsMap[frameCounter] = (timestamp*1000);
        }
    }
#endif
    PHONE = 0;

    //char faceDetector[200] = "/home/developer/other/android_deps/OpenCV-2.4.10-android-sdk/samples/optical-flow/res/raw/lbpcascade_frontalface.xml";
    char faceDetector[200] = "res/raw/lbpcascade_frontalface.xml";
    //char faceDetector[200] = "res/raw/haarcascade_frontalface_alt2.xml";
    switch (method) {
        case METHOD_OPTFLOW:
        optf.setup(faceDetector);
        case METHOD_FARNEBACK:
        farneback.setup(faceDetector);
        break;
        case METHOD_BLACKPIXELS:
        blackpixels.setup(faceDetector);
        break;
        case METHOD_TEMPLATE_BASED:
        templ.setup(faceDetector);
        break;
        case METHOD_BLACK_PIXELS:
        break;
    }

    std::thread t1(captureFrames);
    std::thread t2(doProcessing);

    t1.join();
    t2.join();

    return 0;
}

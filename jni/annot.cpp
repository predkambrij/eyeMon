#include <opencv2/highgui/highgui.hpp>
#include <list>

#include <iostream>
#include <thread>
#include <stdarg.h>  // For va_start, etc.


bool debug_t1_start_log = true;
bool debug_t1_loop_log = false;
bool debug_t2_loop_log = true;
bool debug_t2_show_img_main = true;

char fileName[100] = "/home/developer/other/posnetki/o4_87.mp4"; // na zacetku gledal na sredi ekrana, pol premikal glavo

class FrameCarrier {
    public: cv::Mat frame;
    public: double timestamp;

    public: FrameCarrier(cv::Mat frame, double timestamp) {
        this->frame     = frame;
        this->timestamp = timestamp;

    }
};

std::list<FrameCarrier> frameList;
long unsigned int listSize = 0;

bool finished = false;
bool grabbing = true;
void doLog(bool shouldPrint, const std::string fmt, ...) {
    if (shouldPrint != true) {
        return;
    }
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            break;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    std::cout << str;// << std::endl ;
}
void imshowWrapper(const char* name, cv::Mat mat, bool shouldShow) {
    if (shouldShow == true) {
        imshow(name, mat);
    }
}

int frameGrabber() {
    cv::VideoCapture stream1(fileName);

    if (!stream1.isOpened()) {
        CV_Assert("T1 cam open failed");
    }

    doLog(debug_t1_start_log, "debug_t1_start_log: CAP_PROP_FPS %f\n", stream1.get(CV_CAP_PROP_FPS));
    doLog(debug_t1_start_log, "debug_t1_start_log: CAP_PROP_FRAME_COUNT %f\n", stream1.get(CV_CAP_PROP_FRAME_COUNT));

    cv::Mat frame;
    while (grabbing) {
        if(!(stream1.read(frame))) {
            doLog(debug_t1_loop_log, "debug_t1_loop_log: No captured frame, exiting!\n");
            return 0;
        }
        double frameTimeMs = (double) stream1.get(CV_CAP_PROP_POS_MSEC);
        FrameCarrier fc(frame.clone(), frameTimeMs);
        frameList.push_back(fc);
        listSize++;

        doLog(debug_t1_loop_log, "debug_t1_loop_log: frameTime %lf\n", frameTimeMs);
        // TODO if size > X sleep for a while
        if (listSize > (5*30)) {
            finished = true;
            break;
        }
    }
    finished = true;

    return 0;
}
int frameProcessor() {
    bool pause = false;
    bool forward = true;
    if (debug_t2_show_img_main == true) {
        cv::namedWindow("main",CV_WINDOW_NORMAL); cv::moveWindow("main", 400, 100); cv::resizeWindow("main",1280, 960);
    }
    double frameTimeMs = 0, prevFrameMs = 0;
    std::list<FrameCarrier>::iterator iter = frameList.begin();
    while(iter != frameList.end()) {
        FrameCarrier& fc = *iter;
        cv::Mat frame    = fc.frame;
        frameTimeMs = fc.timestamp;
        if (forward == true) {
            iter++;
        } else {
            iter--;
        }

        // TODO delete old frame from the list
        doLog(debug_t2_loop_log, "debug_t2_loop_log: t %d:%02d.%03d frameTime %lf diff %lf\n", ((int)frameTimeMs)/60000, ((int)frameTimeMs/1000)%60, ((int)frameTimeMs)%1000, frameTimeMs, frameTimeMs-prevFrameMs);
        prevFrameMs = frameTimeMs;

        imshowWrapper("main", frame, debug_t2_show_img_main);
        if (debug_t2_show_img_main == true) {
            // flow control
            int c = cv::waitKey(10);
            if((char)c == 'q') {
                doLog(true, "exiting\n");
                grabbing = false;
                break;
            } else if((char)c == 'p') {
                pause = 1;
            } else if((char)c == 'f') {
                //flg = 1;
                // imwrite("/tmp/frame.png",cflow);
            } else if (pause == 1) {
                while (true) {
                    int c = cv::waitKey(10);
                    if((char)c == 'p') {
                        pause = 0;
                        break;
                    } else if((char)c == 'b') {
                        forward = false;
                    } else if((char)c == 'f') {
                        forward = true;
                    } else if((char)c == 'n') {
                        break;
                    } else if((char)c == 's') {
                        // status
                        //printStatus();

                        break;
                    }
                }
            }
        }
    }
    return 0;
}
int main(int argc, char * argv[]) {
    std::thread t1(frameGrabber);
    std::thread t2(frameProcessor);

    t1.join();
    t2.join();
}

#include <opencv2/highgui/highgui.hpp>
#include <list>

#include <iostream>
#include <thread>
#include <stdarg.h>  // For va_start, etc.


bool debug_t1_start_log = true;
bool debug_t1_loop_log = false;
bool debug_t1_loop_loop_log = false;
bool debug_t2_loop_log = true;
bool debug_t2_show_img_main = true;

//char fileName[100] = "/home/developer/other/posnetki/o4_44.mp4"; // doma
//char fileName[100] = "/home/developer/other/posnetki/o4_87.mp4"; // na zacetku gledal na sredi ekrana, pol premikal glavo
//char fileName[100] = "/home/developer/other/posnetki/o4_89.mp4"; // knjiznica
char fileName[100] = "/home/developer/other/posnetki/o4_90.mp4"; // knjiznica (na zac. gledal na sredo, pol premikal glavo,...)

class FrameCarrier {
    public: cv::Mat frame;
    public: double timestamp;

    public: FrameCarrier(cv::Mat frame, double timestamp) {
        this->frame     = frame;
        this->timestamp = timestamp;

    }
};

std::list<FrameCarrier> frameList;

bool grabbing = true;
long int startPos = 0;
long int grabPos = -1;
long int pos = -1;

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
        grabPos++;
        doLog(debug_t1_loop_log, "debug_t1_loop_log: frame %d frameTime %lf\n", grabPos, frameTimeMs);

        while ((pos + 300) < grabPos && grabbing) {
            doLog(debug_t1_loop_loop_log, "debug_t1_loop_loop_log: sleeping\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    return 0;
}
int frameProcessor() {
    bool pause = false;
    bool forward = true;
    if (debug_t2_show_img_main == true) {
        cv::namedWindow("main",CV_WINDOW_NORMAL); cv::moveWindow("main", -100, 0); cv::resizeWindow("main",640, 480);
        cv::namedWindow("main2",CV_WINDOW_NORMAL); cv::moveWindow("main2", 400, 0); cv::resizeWindow("main2",640, 480);
        cv::namedWindow("main3",CV_WINDOW_NORMAL); cv::moveWindow("main3", 900, 0); cv::resizeWindow("main3",640, 480);
        cv::namedWindow("main4",CV_WINDOW_NORMAL); cv::moveWindow("main4", 1400, 0); cv::resizeWindow("main4",640, 480);
    }

    cv::Mat frame, prevFrame2, prevFrame3, prevFrame4;
    bool prev2FrameAssigned = false;
    bool prev3FrameAssigned = false;
    bool prev4FrameAssigned = false;
    double frameTimeMs = 0, prevFrameMs = 0;
    std::list<FrameCarrier>::iterator iter = frameList.begin();
    FrameCarrier& fc = *iter;

    while(true) {
        if (iter != frameList.end()) {
            fc          = *iter;
            frame       = fc.frame;
            frameTimeMs = fc.timestamp;
        } else {
            doLog(true, "it's end\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (forward == true) {
            doLog(true, "pos %d grabPos %d\n", pos, grabPos);
            if (pos < grabPos) {
                iter++;
                pos++;
            } else {
                doLog(true, "the end\n");
            }
        } else {
            if (startPos < pos) {
                iter--;
                pos--;
            }
        }

        // delete frames which are far behind
        if ((startPos + 300) < pos) {
            frameList.pop_front();
            startPos++;
        }

        doLog(debug_t2_loop_log, "debug_t2_loop_log: t %d:%02d.%03d frameTime %lf diff %lf\n", ((int)frameTimeMs)/60000, ((int)frameTimeMs/1000)%60, ((int)frameTimeMs)%1000, frameTimeMs, frameTimeMs-prevFrameMs);
        prevFrameMs = frameTimeMs;

        imshowWrapper("main", frame, debug_t2_show_img_main);
        if (prev4FrameAssigned) {
            imshowWrapper("main4", prevFrame4, debug_t2_show_img_main);
        }
        if (prev3FrameAssigned) {
            imshowWrapper("main3", prevFrame3, debug_t2_show_img_main);
            prevFrame4 = prevFrame3.clone();
            prev4FrameAssigned = true;
        }
        if (prev2FrameAssigned) {
            imshowWrapper("main2", prevFrame2, debug_t2_show_img_main);
            prevFrame3 = prevFrame2.clone();
            prev3FrameAssigned = true;
        }
        prevFrame2 = frame.clone();
        prev2FrameAssigned = true;

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
int main() { // int argc, char * argv[]
    std::thread t1(frameGrabber);
    std::thread t2(frameProcessor);

    t1.join();
    t2.join();
}

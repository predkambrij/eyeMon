#include <opencv2/highgui/highgui.hpp>

char fileName[200] = "/eyeMon/posnetki/o4_105.mp4";


cv::VideoCapture stream1(fileName);
bool isVideoCapture = false;
bool disable_max_size = false;

// it can load the whole file in seconds and make huge memory consumption
int maxSize = 300;

#define allVid
#ifdef allVid
unsigned int startingFrameNum = 0;
unsigned int endingFrameNum = 0;
#else
// ruka mizo
//unsigned int startingFrameNum = 8900;
//unsigned int endingFrameNum = 9600;

unsigned int startingFrameNum = 0;
unsigned int endingFrameNum = 1000;
#endif


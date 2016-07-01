#include <opencv2/highgui/highgui.hpp>

char fileName[100] = "/home/developer/other/posnetki/talking.avi";


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

// talking blink 56
unsigned int startingFrameNum = 4400;
unsigned int endingFrameNum = 4900;
#endif


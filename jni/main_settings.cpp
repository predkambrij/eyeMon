#include <opencv2/highgui/highgui.hpp>

cv::VideoCapture stream1(0);   //0 is the id of video device.0 if you have only one camera

bool isVideoCapture = true;
bool disable_max_size = false;

int maxSize = 90;

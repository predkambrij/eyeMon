// test videos
// char fileName[100] = "/home/developer/other/posnetki/o4_29.mp4";
// char fileName[100] = "/home/developer/other/posnetki/o4_30.mp4";
//char fileName[100] = "/home/developer/other/posnetki/o4_31.mp4";
// char fileName[200] = "/home/developer/other/test_videos/crnc1.mp4";
// char fileName[200] = "/home/developer/other/test_videos/indian_close.mp4";
// char fileName[200] = "/home/developer/other/test_videos/yellow_close.mp4";
// char fileName[200] = "/home/developer/other/test_videos/very_dark.mp4";
// char fileName[100] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
//char fileName[100] = "/home/developer/other/posnetki/o4_64.mp4"; // 1280x720
//char fileName[100] = "/home/developer/other/posnetki/o4_65.mp4"; // 640x480
//char fileName[100] = "/home/developer/other/posnetki/o4_66.mp4"; // 320x240

#include <opencv2/highgui/highgui.hpp>

//char fileName[100] = "/home/developer/other/posnetki/o4_89.mp4"; // knjiznica
//char fileName[100] = "/home/developer/other/posnetki/o4_90.mp4"; // knjiznica, na zac na sredino (zaflasha svetloba), pol premikal
//char fileName[100] = "/home/developer/other/posnetki/o4_83.mp4";
//char fileName[100] = "/home/developer/other/posnetki/o4_85.mp4"; // pri miru, niso rukal mize
//char fileName[100] = "/home/developer/other/posnetki/o4_86.mp4"; // na zacetku gledal na sredi ekrana
//char fileName[100] = "/home/developer/other/posnetki/o4_87.mp4"; // na zacetku gledal na sredi ekrana, pol premikal glavo
char fileName[100] = "/home/developer/other/posnetki/talking.avi";
cv::VideoCapture stream1(fileName);
bool isVideoCapture = false;
bool disable_max_size = false;

// it can load the whole file in seconds and make huge memory consumption
int maxSize = 300;

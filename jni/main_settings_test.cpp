#include <opencv2/highgui/highgui.hpp>

// test videos
//char fileName[100] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
//char fileName[100] = "/home/developer/other/posnetki/o4_29.mp4";
// char fileName[100] = "/home/developer/other/posnetki/o4_30.mp4";
//char fileName[100] = "/home/developer/other/posnetki/o4_31.mp4";
//char fileName[100] = "/home/developer/other/posnetki/o4_64.mp4"; // 1280x720
//char fileName[100] = "/home/developer/other/posnetki/o4_65.mp4"; // 640x480
//char fileName[100] = "/home/developer/other/posnetki/o4_66.mp4"; // 320x240

//char fileName[200] = "/home/developer/other/posnetki/test_videos/crnc1.mp4";
//char fileName[200] = "/home/developer/other/posnetki/test_videos/very_dark.mp4";
// char fileName[200] = "/home/developer/other/posnetki/test_videos/indian_close.mp4";
// char fileName[200] = "/home/developer/other/posnetki/test_videos/yellow_close.mp4";


//char fileName[100] = "/home/developer/other/posnetki/o4_89.mp4"; // knjiznica
//char fileName[100] = "/home/developer/other/posnetki/o4_90.mp4"; // knjiznica, na zac na sredino (zaflasha svetloba), pol premikal
//char fileName[100] = "/home/developer/other/posnetki/o4_83.mp4";
//char fileName[100] = "/home/developer/other/posnetki/o4_85.mp4"; // pri miru, niso rukal mize
//char fileName[100] = "/home/developer/other/posnetki/o4_86.mp4"; // na zacetku gledal na sredi ekrana
//char fileName[100] = "/home/developer/other/posnetki/o4_87.mp4"; // na zacetku gledal na sredi ekrana, pol premikal glavo
//char fileName[100] = "/home/developer/other/posnetki/o4_96.mp4"; // svetloba s strani
//char fileName[100] = "/home/developer/other/posnetki/o4_97.mp4"; // svetloba od zadej knjiznica fri
//char fileName[100] = "/home/developer/other/posnetki/o4_98.mp4"; // svetloba od strani knjiznica fri spodaj
//char fileName[100] = "/home/developer/other/posnetki/o4_99.mp4"; // svetloba od zadaj knjiznica (veliko suma)

//char fileName[100] = "/home/developer/other/posnetki/talking.avi";
//char fileName[100] = "/home/developer/other/posnetki/sk/eyeblink8/1/26122013_223310_cam.avi"; // punca od dalec
//char fileName[100] = "/home/developer/other/posnetki/sk/NightOfResearchers15/test/14/26092014_211047_cam.avi"; // ful partial

// challenging
char fileName[100] = "/home/developer/other/posnetki/sk/NightOfResearchers30/test/1/26092014_161749_cam.avi"; // kitajka
//char fileName[100] = "/home/developer/other/posnetki/sk/NightOfResearchers15/trainval/train/4/26092014_182617_cam.avi"; // moski ocala
//char fileName[100] = "/home/developer/other/posnetki/sk/NightOfResearchers30/test/11/26092014_173630_cam.avi"; // zenska ocala





cv::VideoCapture stream1(fileName);
bool isVideoCapture = false;
bool disable_max_size = false;

// it can load the whole file in seconds and make huge memory consumption
int maxSize = 300;

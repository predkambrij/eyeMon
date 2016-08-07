#include <opencv2/highgui/highgui.hpp>

// test videos
//char fileName[200] = "/opt/docker_volumes/mag/home_developer/other/posnetki/o4_29.mp4";
//char fileName[200] = "/home/developer/other/posnetki/o4_29.mp4";
// char fileName[200] = "/home/developer/other/posnetki/o4_30.mp4";
//char fileName[200] = "/home/developer/other/posnetki/o4_31.mp4";
//char fileName[200] = "/home/developer/other/posnetki/o4_64.mp4"; // 1280x720
//char fileName[200] = "/home/developer/other/posnetki/o4_65.mp4"; // 640x480
//char fileName[200] = "/home/developer/other/posnetki/o4_66.mp4"; // 320x240

//char fileName[200] = "/home/developer/other/posnetki/test_videos/crnc1.mp4";
//char fileName[200] = "/home/developer/other/posnetki/test_videos/very_dark.mp4";
// char fileName[200] = "/home/developer/other/posnetki/test_videos/indian_close.mp4";
// char fileName[200] = "/home/developer/other/posnetki/test_videos/yellow_close.mp4";


//char fileName[200] = "/home/developer/other/posnetki/o4_89.mp4"; // knjiznica
//char fileName[200] = "/home/developer/other/posnetki/o4_90.mp4"; // knjiznica, na zac na sredino (zaflasha svetloba), pol premikal
//char fileName[200] = "/home/developer/other/posnetki/o4_83.mp4";
//char fileName[200] = "/home/developer/other/posnetki/o4_85.mp4"; // pri miru, niso rukal mize
//char fileName[200] = "/home/developer/other/posnetki/o4_86.mp4"; // na zacetku gledal na sredi ekrana
//char fileName[200] = "/home/developer/other/posnetki/o4_87.mp4"; // na zacetku gledal na sredi ekrana, pol premikal glavo
//char fileName[200] = "/home/developer/other/posnetki/o4_96.mp4"; // svetloba s strani
//char fileName[200] = "/home/developer/other/posnetki/o4_97.mp4"; // svetloba od zadej knjiznica fri
//char fileName[200] = "/home/developer/other/posnetki/o4_98.mp4"; // svetloba od strani knjiznica fri spodaj
//char fileName[200] = "/home/developer/other/posnetki/o4_99.mp4"; // svetloba od zadaj knjiznica (veliko suma)
//char fileName[200] = "/home/developer/other/posnetki/o4_100.mp4"; // svetloba od zadaj knjiznica (zelo veliko suma)

// my annots
//char fileName[200] = "/home/developer/other/posnetki/o4_44.mp4"; // doma iv
//char fileName[200] = "/home/developer/other/posnetki/o4_89.mp4"; // knjiznica
//char fileName[200] = "/home/developer/other/posnetki/o4_90.mp4"; // knjiznica luc zadi, premikal glavo
//char fileName[200] = "/home/developer/other/posnetki/o4_101.mp4"; // iso noise

//char fileName[200] = "/home/developer/other/posnetki/sk/basler/poli1person6/poli1person6_x263.avi";

//char fileName[200] = "/home/developer/other/posnetki/o4_102.mp4"; // look away
//char fileName[200] = "/home/developer/other/posnetki/phone1.mp4"; // phone1


//char fileName[200] = "/home/developer/other/posnetki/sk/eyeblink8/3/26122013_230103_cam.avi"; // fant gleda neki

char fileName[200] = "/eyeMon/posnetki/talking.avi";
//char fileName[200] = "/home/developer/other/posnetki/talking.avi";
//char fileName[200] = "/home/developer/other/posnetki/sk/eyeblink8/1/26122013_223310_cam.avi"; // punca od dalec
//char fileName[200] = "/home/developer/other/posnetki/sk/NightOfResearchers15/test/14/26092014_211047_cam.avi"; // ful partial
//char fileName[200] = "/home/developer/other/posnetki/sk/NightOfResearchers30/test/9/26092014_172120_cam.avi";
//char fileName[200] = "/home/developer/other/posnetki/sk/eyeblink8/11/27122013_154548_cam.avi";

// challenging
//char fileName[200] = "/home/developer/other/posnetki/sk/NightOfResearchers30/test/1/26092014_161749_cam.avi"; // kitajka
//char fileName[200] = "/home/developer/other/posnetki/sk/NightOfResearchers15/trainval/train/4/26092014_182617_cam.avi"; // moski ocala
//char fileName[200] = "/home/developer/other/posnetki/sk/NightOfResearchers30/test/11/26092014_173630_cam.avi"; // zenska ocala

//char fileName[200] = "/home/developer/other/posnetki/sk/eyeblink8/2/26122013_224532_cam.avi"; // hand in the first frame
//char fileName[200] = "/home/developer/other/posnetki/sk/zju/66/000017F_FTN.avi";

//char fileName[200] = "/home/developer/other/posnetki/sk/eyeblink8/10/27122013_153916_cam.avi"; // dobr rezultat



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
unsigned int startingFrameNum = 800;
unsigned int endingFrameNum = 0;
#endif
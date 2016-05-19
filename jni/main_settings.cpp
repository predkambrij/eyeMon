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

//#define VID_CAP

#ifndef VID_CAP
char fileName[100] = "/home/developer/other/posnetki/o4_44.mp4";

VideoCapture stream1(fileName);   //0 is the id of video device.0 if you have only one camera

bool isVideoCapture = false;
bool disable_max_size = true;

#else
VideoCapture stream1(0);
bool isVideoCapture = true;
bool disable_max_size = false;

#endif

int maxSize = 300;

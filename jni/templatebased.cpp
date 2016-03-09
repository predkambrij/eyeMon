#include <opencv2/objdetect/objdetect.hpp>

#include <common.hpp>

using namespace cv;
using namespace std;

class TemplateBased {
    cv::CascadeClassifier face_cascade;

    public: int setup(const char* cascadeFileName) {
        try {
            if(!face_cascade.load(cascadeFileName)) {
                throw "--(!)Error loading face cascade, please change face_cascade_name in source code.\n";
            }
        } catch (const char* msg) {
            doLog(msg);
            throw;
        }
        return 0;
    }
#ifdef IS_PHONE
    public: int setJni(JNIEnv* jenv) {
    }
#endif
    public: int run(Mat gray, Mat out) {
    }

}; // end of TemplateBased class definition

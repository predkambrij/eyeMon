#include <common.hpp>

// TODO this could be done using macros (at all places is used ifdef)
bool debug_show_img_main = true;
bool debug_show_img_face = false;
bool debug_show_img_optfl_eyes = false;
bool debug_show_img_templ_eyes_cor = false;
bool debug_show_img_templ_eyes_tmpl = false;
bool debug_t1_log = false;
bool debug_t2_log = false;
bool debug_t2_perf_method = false;
bool debug_t2_perf_whole = true;
bool debug_tmpl_log = false;
bool debug_tmpl_perf1 = false;
bool debug_tmpl_perf2 = false;
bool debug_notifications_log1 = false;
bool debug_blinks_d1 = false;
bool debug_blinks_d2 = false;
bool debug_blinks_d3 = false;
bool debug_blinks_d4 = false;

//int method = METHOD_OPTFLOW;
int method = METHOD_TEMPLATE_BASED;
double previousFrameTime = -1;

// optical flow
int flg=0;
///

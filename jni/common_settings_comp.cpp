#include <common.hpp>

// TODO this could be done using macros (at all places is used ifdef)
bool debug_show_img_main = true;
bool debug_show_img_face = true;
bool debug_show_img_templ_eyes_cor = true;
bool debug_show_img_templ_eyes_tmpl = true;
bool debug_show_img_optfl_eyes = false;
bool debug_t1_log = false;
bool debug_t2_log = true;
bool debug_t2_perf_method = true;
bool debug_t2_perf_whole = true;
bool debug_tmpl_log = true;
bool debug_tmpl_perf1 = true;
bool debug_tmpl_perf2 = true;
bool debug_notifications_log1 = true;
bool debug_blinks_d1 = true;
bool debug_blinks_d2 = true;
bool debug_blinks_d3 = true;
bool debug_blinks_d4 = true;

//int method = METHOD_OPTFLOW;
int method = METHOD_TEMPLATE_BASED;
double previousFrameTime = -1;

// optical flow
int flg=0;
///

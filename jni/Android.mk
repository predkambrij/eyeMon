LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#OPENCV_CAMERA_MODULES:=off
#OPENCV_INSTALL_MODULES:=off
#OPENCV_LIB_TYPE:=SHARED
#include ../../sdk/native/jni/OpenCV.mk
include ../../../OpenCV-2.4.10-android-sdk/sdk/native/jni/OpenCV.mk
#include ../../../OpenCV-android-sdk-2-4-11/sdk/native/jni/OpenCV.mk

LOCAL_SRC_FILES  := common_settings_phone.cpp common.cpp blinkmeasure.cpp optflow_jni.cpp optflow.cpp templatebased_jni.cpp eyeLike/src/findEyeCenter.cpp eyeLike/src/helpers.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_LDLIBS     += -llog -ldl

LOCAL_CFLAGS += -std=c++11 -DIS_PHONE

LOCAL_MODULE     := eyemon

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
OPENCV_LIB_TYPE:=SHARED
OPENCV_INSTALL_MODULES:=on
OPENCV_CAMERA_MODULES:=on

include jni/jni/OpenCV.mk

LOCAL_MODULE    := app
LOCAL_SRC_FILES := main.cpp

LOCAL_STATIC_LIBRARIES += libopencv_contrib libopencv_legacy libopencv_ml libopencv_stitching libopencv_nonfree libopencv_objdetect libopencv_videostab libopencv_calib3d libopencv_photo libopencv_video libopencv_features2d libopencv_highgui libopencv_androidcamera libopencv_flann libopencv_imgproc libopencv_ts libopencv_core

LOCAL_LDLIBS +=  -llog -ldl
include $(BUILD_SHARED_LIBRARY)

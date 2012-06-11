LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := threading
LOCAL_SRC_FILES := com_Threading_ThreadActivity.c
LOCAL_LDLIBS    += -landroid
LOCAL_LDLIBS    += -llog
include $(BUILD_SHARED_LIBRARY)
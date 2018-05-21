LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)  
LOCAL_MODULE := inject   
LOCAL_SRC_FILES := \
            inject.c
  
LOCAL_LDLIBS += -llog  
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)  
LOCAL_MODULE := qever   
LOCAL_SRC_FILES := \
            qever.c
  
LOCAL_LDLIBS += -llog  -pthread
  
include $(BUILD_SHARED_LIBRARY)
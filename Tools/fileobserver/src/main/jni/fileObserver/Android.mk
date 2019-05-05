LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS:=-L$(SYSROOT)/usr/lib -llog
LOCAL_MODULE := fileobserver
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_SRC_FILES :=  \
    $(LOCAL_PATH)/src/fileObserver.c

include $(BUILD_SHARED_LIBRARY)


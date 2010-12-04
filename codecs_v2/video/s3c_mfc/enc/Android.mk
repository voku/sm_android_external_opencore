LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/SsbSipH264Encode.c \
	src/SsbSipMpeg4Encode.c \
	src/SsbSipLogMsg.c

LOCAL_MODULE := libmfcencapi

LOCAL_CFLAGS :=   $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)//codecs_v2/video/s3c_mfc/enc/include \
	$(PV_TOP)//codecs_v2/video/s3c_mfc/enc/src \
	$(PV_INCLUDES) 

	
LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)
LOCAL_COPY_HEADERS := \
	include/SsbSipH264Encode.h \
	include/SsbSipMpeg4Encode.h

BOARD_OPENCORE_LIBRARIES := libmfcencapi

include $(BUILD_STATIC_LIBRARY)


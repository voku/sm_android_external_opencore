LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libomx_m4v_component_lib \
	libmfcdecapi

LOCAL_MODULE := libomx_m4vdec_sharedlibrary

-include $(PV_TOP)/Android_platform_extras.mk

-include $(PV_TOP)/Android_system_extras.mk

LOCAL_SHARED_LIBRARIES +=   libomx_sharedlibrary libopencore_common 

include $(BUILD_SHARED_LIBRARY)
include   $(PV_TOP)/codecs_v2/omx/omx_m4v/Android.mk
include   $(PV_TOP)/codecs_v2/video/s3c_mfc/dec/Android.mk


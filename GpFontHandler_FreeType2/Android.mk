LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := GpFontHandler_FreeType2

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../Common	\
	$(LOCAL_PATH)/../GpCommon	\
	$(LOCAL_PATH)/../FreeType/freetype/include

LOCAL_CFLAGS := -DGP_DEBUG_CONFIG=0

# Add your application source files here...
LOCAL_SRC_FILES := \
	GpFontHandler_FreeType2.cpp

LOCAL_SHARED_LIBRARIES := FreeType

include $(BUILD_STATIC_LIBRARY)

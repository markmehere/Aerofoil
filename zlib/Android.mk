LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := zlib

# Add your application source files here...
LOCAL_SRC_FILES := \
	adler32.c	\
	crc32.c	\
	deflate.c	\
	inffast.c	\
	inflate.c	\
	inftrees.c	\
	trees.c	\
	zutil.c
	

include $(BUILD_STATIC_LIBRARY)

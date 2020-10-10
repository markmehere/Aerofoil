LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := FreeType

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/freetype/include

# Add your application source files here...
LOCAL_SRC_FILES := \
	$(LOCAL_PATH)/freetype/src/base/ftbase.c	\
	$(LOCAL_PATH)/freetype/src/base/ftbitmap.c	\
	$(LOCAL_PATH)/freetype/src/base/ftdebug.c	\
	$(LOCAL_PATH)/freetype/src/base/ftinit.c	\
	$(LOCAL_PATH)/freetype/src/base/ftsystem.c	\
	$(LOCAL_PATH)/freetype/src/winfonts/winfnt.c	\
	$(LOCAL_PATH)/freetype/src/autofit/autofit.c	\
	$(LOCAL_PATH)/freetype/src/bdf/bdf.c	\
	$(LOCAL_PATH)/freetype/src/cff/cff.c	\
	$(LOCAL_PATH)/freetype/src/gzip/ftgzip.c	\
	$(LOCAL_PATH)/freetype/src/lzw/ftlzw.c	\
	$(LOCAL_PATH)/freetype/src/pcf/pcf.c	\
	$(LOCAL_PATH)/freetype/src/pfr/pfr.c	\
	$(LOCAL_PATH)/freetype/src/psaux/psaux.c	\
	$(LOCAL_PATH)/freetype/src/pshinter/pshinter.c	\
	$(LOCAL_PATH)/freetype/src/psnames/psnames.c	\
	$(LOCAL_PATH)/freetype/src/raster/raster.c	\
	$(LOCAL_PATH)/freetype/src/sfnt/sfnt.c	\
	$(LOCAL_PATH)/freetype/src/smooth/smooth.c	\
	$(LOCAL_PATH)/freetype/src/truetype/truetype.c	\
	$(LOCAL_PATH)/freetype/src/type1/type1.c	\
	$(LOCAL_PATH)/freetype/src/cid/type1cid.c	\
	$(LOCAL_PATH)/freetype/src/type42/type42.c

LOCAL_CFLAGS := -DFT2_BUILD_LIBRARY

include $(BUILD_SHARED_LIBRARY)

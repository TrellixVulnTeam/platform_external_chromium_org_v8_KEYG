# This file is generated by gyp; do not edit.

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE := v8_tools_gyp_mksnapshot_ia32_host_gyp
LOCAL_MODULE_STEM := mksnapshot.ia32
LOCAL_MODULE_SUFFIX := 
LOCAL_MODULE_TAGS := optional
LOCAL_IS_HOST_MODULE := true
gyp_intermediate_dir := $(call local-intermediates-dir)
gyp_shared_intermediate_dir := $(call intermediates-dir-for,GYP,shared)

# Make sure our deps are built first.
GYP_TARGET_DEPENDENCIES := \
	$(call intermediates-dir-for,STATIC_LIBRARIES,v8_tools_gyp_v8_base_ia32_host_gyp,true)/v8_tools_gyp_v8_base_ia32_host_gyp.a \
	$(call intermediates-dir-for,STATIC_LIBRARIES,v8_tools_gyp_v8_nosnapshot_ia32_host_gyp,true)/v8_tools_gyp_v8_nosnapshot_ia32_host_gyp.a \
	$(call intermediates-dir-for,GYP,v8_tools_gyp_js2c_host_gyp,true)/js2c.stamp

GYP_GENERATED_OUTPUTS :=

# Make sure our deps and generated files are built first.
LOCAL_ADDITIONAL_DEPENDENCIES := $(GYP_TARGET_DEPENDENCIES) $(GYP_GENERATED_OUTPUTS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_GENERATED_SOURCES :=

GYP_COPIED_SOURCE_ORIGIN_DIRS :=

LOCAL_SRC_FILES := \
	v8/src/mksnapshot.cc


# Flags passed to both C and C++ files.
MY_CFLAGS := \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	-pthread \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-fPIC \
	-Wno-format \
	-m32 \
	-Os \
	-g \
	-fomit-frame-pointer \
	-fdata-sections \
	-ffunction-sections

MY_CFLAGS_C :=

MY_DEFS := \
	'-D_FILE_OFFSET_BITS=64' \
	'-DUSE_LINUX_BREAKPAD' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_GPU=1' \
	'-DUSE_OPENSSL=1' \
	'-DENABLE_EGLIMAGE=1' \
	'-DENABLE_LANGUAGE_DETECTION=1' \
	'-DENABLE_DEBUGGER_SUPPORT' \
	'-DV8_TARGET_ARCH_IA32' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=1' \
	'-DWTF_USE_DYNAMIC_ANNOTATIONS=1' \
	'-D_DEBUG' \
	'-DENABLE_DISASSEMBLER' \
	'-DV8_ENABLE_CHECKS' \
	'-DOBJECT_PRINT' \
	'-DVERIFY_HEAP' \
	'-DENABLE_EXTRA_CHECKS'

LOCAL_CFLAGS := $(MY_CFLAGS_C) $(MY_CFLAGS) $(MY_DEFS)
# Undefine ANDROID for host modules
LOCAL_CFLAGS += -UANDROID

# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/v8/src

LOCAL_C_INCLUDES := $(GYP_COPIED_SOURCE_ORIGIN_DIRS) $(LOCAL_C_INCLUDES)

# Flags passed to only C++ (and not C) files.
LOCAL_CPPFLAGS := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated

### Rules for final target.

LOCAL_LDFLAGS := \
	-pthread \
	-fPIC \
	-m32


LOCAL_STATIC_LIBRARIES := \
	v8_tools_gyp_v8_base_ia32_host_gyp \
	v8_tools_gyp_v8_nosnapshot_ia32_host_gyp

# Enable grouping to fix circular references
LOCAL_GROUP_STATIC_LIBRARIES := true

LOCAL_SHARED_LIBRARIES :=

# Add target alias to "gyp_all_modules" target.
.PHONY: gyp_all_modules
gyp_all_modules: v8_tools_gyp_mksnapshot_ia32_host_gyp

# Alias gyp target name.
.PHONY: mksnapshot.ia32
mksnapshot.ia32: v8_tools_gyp_mksnapshot_ia32_host_gyp

LOCAL_MODULE_PATH := $(gyp_shared_intermediate_dir)
include $(BUILD_HOST_EXECUTABLE)

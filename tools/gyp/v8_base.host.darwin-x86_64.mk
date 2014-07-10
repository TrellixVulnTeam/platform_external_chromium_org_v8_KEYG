# This file is generated by gyp; do not edit.

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE := v8_tools_gyp_v8_base_$(TARGET_$(GYP_VAR_PREFIX)ARCH)_host_gyp
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_TAGS := optional
LOCAL_IS_HOST_MODULE := true
LOCAL_MULTILIB := $(GYP_HOST_MULTILIB)
gyp_intermediate_dir := $(call local-intermediates-dir,,$(GYP_HOST_VAR_PREFIX))
gyp_shared_intermediate_dir := $(call intermediates-dir-for,GYP,shared,,,$(GYP_VAR_PREFIX))

# Make sure our deps are built first.
GYP_TARGET_DEPENDENCIES :=

GYP_GENERATED_OUTPUTS :=

# Make sure our deps and generated files are built first.
LOCAL_ADDITIONAL_DEPENDENCIES := $(GYP_TARGET_DEPENDENCIES) $(GYP_GENERATED_OUTPUTS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_GENERATED_SOURCES :=

GYP_COPIED_SOURCE_ORIGIN_DIRS :=

LOCAL_SRC_FILES := \
	v8/src/accessors.cc \
	v8/src/allocation.cc \
	v8/src/allocation-site-scopes.cc \
	v8/src/allocation-tracker.cc \
	v8/src/api.cc \
	v8/src/arguments.cc \
	v8/src/assembler.cc \
	v8/src/assert-scope.cc \
	v8/src/ast-value-factory.cc \
	v8/src/ast.cc \
	v8/src/bignum-dtoa.cc \
	v8/src/bignum.cc \
	v8/src/bootstrapper.cc \
	v8/src/builtins.cc \
	v8/src/cached-powers.cc \
	v8/src/checks.cc \
	v8/src/code-stubs.cc \
	v8/src/code-stubs-hydrogen.cc \
	v8/src/codegen.cc \
	v8/src/compilation-cache.cc \
	v8/src/compiler.cc \
	v8/src/contexts.cc \
	v8/src/conversions.cc \
	v8/src/counters.cc \
	v8/src/cpu-profiler.cc \
	v8/src/data-flow.cc \
	v8/src/date.cc \
	v8/src/dateparser.cc \
	v8/src/debug.cc \
	v8/src/deoptimizer.cc \
	v8/src/disassembler.cc \
	v8/src/diy-fp.cc \
	v8/src/dtoa.cc \
	v8/src/elements-kind.cc \
	v8/src/elements.cc \
	v8/src/execution.cc \
	v8/src/extensions/externalize-string-extension.cc \
	v8/src/extensions/free-buffer-extension.cc \
	v8/src/extensions/gc-extension.cc \
	v8/src/extensions/statistics-extension.cc \
	v8/src/extensions/trigger-failure-extension.cc \
	v8/src/factory.cc \
	v8/src/fast-dtoa.cc \
	v8/src/field-index.cc \
	v8/src/fixed-dtoa.cc \
	v8/src/flags.cc \
	v8/src/frames.cc \
	v8/src/full-codegen.cc \
	v8/src/func-name-inferrer.cc \
	v8/src/gdb-jit.cc \
	v8/src/global-handles.cc \
	v8/src/handles.cc \
	v8/src/heap-profiler.cc \
	v8/src/heap-snapshot-generator.cc \
	v8/src/heap.cc \
	v8/src/hydrogen-bce.cc \
	v8/src/hydrogen-bch.cc \
	v8/src/hydrogen-canonicalize.cc \
	v8/src/hydrogen-check-elimination.cc \
	v8/src/hydrogen-dce.cc \
	v8/src/hydrogen-dehoist.cc \
	v8/src/hydrogen-environment-liveness.cc \
	v8/src/hydrogen-escape-analysis.cc \
	v8/src/hydrogen-instructions.cc \
	v8/src/hydrogen.cc \
	v8/src/hydrogen-gvn.cc \
	v8/src/hydrogen-infer-representation.cc \
	v8/src/hydrogen-infer-types.cc \
	v8/src/hydrogen-load-elimination.cc \
	v8/src/hydrogen-mark-deoptimize.cc \
	v8/src/hydrogen-mark-unreachable.cc \
	v8/src/hydrogen-osr.cc \
	v8/src/hydrogen-range-analysis.cc \
	v8/src/hydrogen-redundant-phi.cc \
	v8/src/hydrogen-removable-simulates.cc \
	v8/src/hydrogen-representation-changes.cc \
	v8/src/hydrogen-sce.cc \
	v8/src/hydrogen-store-elimination.cc \
	v8/src/hydrogen-types.cc \
	v8/src/hydrogen-uint32-analysis.cc \
	v8/src/i18n.cc \
	v8/src/icu_util.cc \
	v8/src/ic.cc \
	v8/src/incremental-marking.cc \
	v8/src/interface.cc \
	v8/src/interpreter-irregexp.cc \
	v8/src/isolate.cc \
	v8/src/jsregexp.cc \
	v8/src/lithium-allocator.cc \
	v8/src/lithium-codegen.cc \
	v8/src/lithium.cc \
	v8/src/liveedit.cc \
	v8/src/log-utils.cc \
	v8/src/log.cc \
	v8/src/lookup.cc \
	v8/src/mark-compact.cc \
	v8/src/messages.cc \
	v8/src/objects-debug.cc \
	v8/src/objects-printer.cc \
	v8/src/objects-visiting.cc \
	v8/src/objects.cc \
	v8/src/optimizing-compiler-thread.cc \
	v8/src/ostreams.cc \
	v8/src/parser.cc \
	v8/src/perf-jit.cc \
	v8/src/preparse-data.cc \
	v8/src/preparser.cc \
	v8/src/prettyprinter.cc \
	v8/src/profile-generator.cc \
	v8/src/property.cc \
	v8/src/regexp-macro-assembler-irregexp.cc \
	v8/src/regexp-macro-assembler-tracer.cc \
	v8/src/regexp-macro-assembler.cc \
	v8/src/regexp-stack.cc \
	v8/src/rewriter.cc \
	v8/src/runtime-profiler.cc \
	v8/src/runtime.cc \
	v8/src/safepoint-table.cc \
	v8/src/sampler.cc \
	v8/src/scanner-character-streams.cc \
	v8/src/scanner.cc \
	v8/src/scopeinfo.cc \
	v8/src/scopes.cc \
	v8/src/serialize.cc \
	v8/src/snapshot-source-sink.cc \
	v8/src/spaces.cc \
	v8/src/store-buffer.cc \
	v8/src/string-search.cc \
	v8/src/string-stream.cc \
	v8/src/strtod.cc \
	v8/src/stub-cache.cc \
	v8/src/sweeper-thread.cc \
	v8/src/token.cc \
	v8/src/transitions.cc \
	v8/src/type-info.cc \
	v8/src/types.cc \
	v8/src/typing.cc \
	v8/src/unicode.cc \
	v8/src/utils.cc \
	v8/src/v8.cc \
	v8/src/v8threads.cc \
	v8/src/variables.cc \
	v8/src/version.cc \
	v8/src/zone.cc \
	v8/src/x64/assembler-x64.cc \
	v8/src/x64/builtins-x64.cc \
	v8/src/x64/code-stubs-x64.cc \
	v8/src/x64/codegen-x64.cc \
	v8/src/x64/cpu-x64.cc \
	v8/src/x64/debug-x64.cc \
	v8/src/x64/deoptimizer-x64.cc \
	v8/src/x64/disasm-x64.cc \
	v8/src/x64/frames-x64.cc \
	v8/src/x64/full-codegen-x64.cc \
	v8/src/x64/ic-x64.cc \
	v8/src/x64/lithium-codegen-x64.cc \
	v8/src/x64/lithium-gap-resolver-x64.cc \
	v8/src/x64/lithium-x64.cc \
	v8/src/x64/macro-assembler-x64.cc \
	v8/src/x64/regexp-macro-assembler-x64.cc \
	v8/src/x64/stub-cache-x64.cc


# Flags passed to both C and C++ files.
MY_CFLAGS_Debug := \
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
	-m64 \
	-Os \
	-g \
	-fdata-sections \
	-ffunction-sections \
	-fomit-frame-pointer \
	-funwind-tables

MY_DEFS_Debug := \
	'-DV8_DEPRECATION_WARNINGS' \
	'-DBLINK_SCALE_FILTERS_AT_RECORD_TIME' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_BROWSER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DDISCARDABLE_MEMORY_ALWAYS_SUPPORTED_NATIVELY' \
	'-DSYSTEM_NATIVELY_SIGNALS_MEMORY_PRESSURE' \
	'-DENABLE_EGLIMAGE=1' \
	'-DCLD_VERSION=1' \
	'-DCLD_DATA_FROM_STATIC' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DDATA_REDUCTION_FALLBACK_HOST="http://compress.googlezip.net:80/"' \
	'-DDATA_REDUCTION_DEV_HOST="http://proxy-dev.googlezip.net:80/"' \
	'-DSPDY_PROXY_AUTH_ORIGIN="https://proxy.googlezip.net:443/"' \
	'-DDATA_REDUCTION_PROXY_PROBE_URL="http://check.googlezip.net/connect"' \
	'-DDATA_REDUCTION_PROXY_WARMUP_URL="http://www.gstatic.com/generate_204"' \
	'-DVIDEO_HOLE=1' \
	'-DV8_TARGET_ARCH_X64' \
	'-DV8_I18N_SUPPORT' \
	'-DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_STATIC' \
	'-DU_USING_ICU_NAMESPACE=0' \
	'-DU_STATIC_IMPLEMENTATION' \
	'-DUSE_OPENSSL=1' \
	'-DUSE_OPENSSL_CERTS=1' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=1' \
	'-DWTF_USE_DYNAMIC_ANNOTATIONS=1' \
	'-D_DEBUG' \
	'-DENABLE_DISASSEMBLER' \
	'-DV8_ENABLE_CHECKS' \
	'-DOBJECT_PRINT' \
	'-DVERIFY_HEAP' \
	'-DENABLE_EXTRA_CHECKS' \
	'-DENABLE_HANDLE_ZAPPING'


# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES_Debug := \
	$(LOCAL_PATH)/v8 \
	$(gyp_shared_intermediate_dir) \
	$(LOCAL_PATH)/third_party/icu/source/i18n \
	$(LOCAL_PATH)/third_party/icu/source/common


# Flags passed to only C++ (and not C) files.
LOCAL_CPPFLAGS_Debug := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated


# Flags passed to both C and C++ files.
MY_CFLAGS_Release := \
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
	-m64 \
	-fno-ident \
	-fdata-sections \
	-ffunction-sections \
	-fomit-frame-pointer \
	-funwind-tables \
	-fdata-sections \
	-ffunction-sections \
	-O2

MY_DEFS_Release := \
	'-DV8_DEPRECATION_WARNINGS' \
	'-DBLINK_SCALE_FILTERS_AT_RECORD_TIME' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_BROWSER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DDISCARDABLE_MEMORY_ALWAYS_SUPPORTED_NATIVELY' \
	'-DSYSTEM_NATIVELY_SIGNALS_MEMORY_PRESSURE' \
	'-DENABLE_EGLIMAGE=1' \
	'-DCLD_VERSION=1' \
	'-DCLD_DATA_FROM_STATIC' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DDATA_REDUCTION_FALLBACK_HOST="http://compress.googlezip.net:80/"' \
	'-DDATA_REDUCTION_DEV_HOST="http://proxy-dev.googlezip.net:80/"' \
	'-DSPDY_PROXY_AUTH_ORIGIN="https://proxy.googlezip.net:443/"' \
	'-DDATA_REDUCTION_PROXY_PROBE_URL="http://check.googlezip.net/connect"' \
	'-DDATA_REDUCTION_PROXY_WARMUP_URL="http://www.gstatic.com/generate_204"' \
	'-DVIDEO_HOLE=1' \
	'-DV8_TARGET_ARCH_X64' \
	'-DV8_I18N_SUPPORT' \
	'-DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_STATIC' \
	'-DU_USING_ICU_NAMESPACE=0' \
	'-DU_STATIC_IMPLEMENTATION' \
	'-DUSE_OPENSSL=1' \
	'-DUSE_OPENSSL_CERTS=1' \
	'-DNDEBUG' \
	'-DNVALGRIND' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=0' \
	'-DENABLE_HANDLE_ZAPPING'


# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES_Release := \
	$(LOCAL_PATH)/v8 \
	$(gyp_shared_intermediate_dir) \
	$(LOCAL_PATH)/third_party/icu/source/i18n \
	$(LOCAL_PATH)/third_party/icu/source/common


# Flags passed to only C++ (and not C) files.
LOCAL_CPPFLAGS_Release := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wno-deprecated


LOCAL_CFLAGS := $(MY_CFLAGS_$(GYP_CONFIGURATION)) $(MY_DEFS_$(GYP_CONFIGURATION))
# Undefine ANDROID for host modules
LOCAL_CFLAGS += -UANDROID
LOCAL_C_INCLUDES := $(GYP_COPIED_SOURCE_ORIGIN_DIRS) $(LOCAL_C_INCLUDES_$(GYP_CONFIGURATION))
LOCAL_CPPFLAGS := $(LOCAL_CPPFLAGS_$(GYP_CONFIGURATION))
LOCAL_ASFLAGS := $(LOCAL_CFLAGS)
### Rules for final target.

LOCAL_LDFLAGS_Debug := \
	-pthread \
	-fPIC \
	-m64


LOCAL_LDFLAGS_Release := \
	-pthread \
	-fPIC \
	-m64


LOCAL_LDFLAGS := $(LOCAL_LDFLAGS_$(GYP_CONFIGURATION))

LOCAL_STATIC_LIBRARIES :=

# Enable grouping to fix circular references
LOCAL_GROUP_STATIC_LIBRARIES := true

LOCAL_SHARED_LIBRARIES :=

# Add target alias to "gyp_all_modules" target.
.PHONY: gyp_all_modules
gyp_all_modules: v8_tools_gyp_v8_base_$(TARGET_$(GYP_VAR_PREFIX)ARCH)_host_gyp

# Alias gyp target name.
.PHONY: v8_base
v8_base: v8_tools_gyp_v8_base_$(TARGET_$(GYP_VAR_PREFIX)ARCH)_host_gyp

include $(BUILD_HOST_STATIC_LIBRARY)

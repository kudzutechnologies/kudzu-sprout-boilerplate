ifdef CONFIG_OPTIMIZATION_LEVEL_RELEASE
  ifdef CONFIG_LOG_DEFAULT_LEVEL_DEBUG
    COMPONENT_ADD_LDFLAGS := $(COMPONENT_PATH)/lib/libKernel-rd.a
  else
    COMPONENT_ADD_LDFLAGS := $(COMPONENT_PATH)/lib/libKernel-ri.a
  endif
else
  ifdef CONFIG_LOG_DEFAULT_LEVEL_DEBUG
    COMPONENT_ADD_LDFLAGS := $(COMPONENT_PATH)/lib/libKernel-dd.a
  else
    COMPONENT_ADD_LDFLAGS := $(COMPONENT_PATH)/lib/libKernel-di.a
  endif
endif
COMPONENT_ADD_INCLUDEDIRS := include
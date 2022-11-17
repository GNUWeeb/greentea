#
# SPDX-License-Identifier: GPL-2.0-only
#
# @author Ammar Faizi <ammarfaizi2@gnuweeb.org> https://www.facebook.com/ammarfaizi2
# @license GPL-2.0-only
#
# Copyright (C) 2022 Ammar Faizi <ammarfaizi2@gnuweeb.org>
#

VERSION	= 0
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION :=
NAME = Green Grass
TARGET_BIN = greentea

MKDIR		:= mkdir
BASE_DIR	:= $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
BASE_DIR	:= $(strip $(patsubst %/, %, $(BASE_DIR)))
BASE_DEP_DIR	:= $(BASE_DIR)/.deps
MAKEFILE_FILE	:= $(lastword $(MAKEFILE_LIST))
INCLUDE_DIR	= -I$(BASE_DIR)
PACKAGE_NAME	:= $(TARGET_BIN)-$(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)

#
# CCXXFLAGS contains flags that are applied to
# CFLAGS and CXXFLAGS.
#
CCXXFLAGS := \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-fpic \
	-fPIC \
	-ggdb3 \
	-O2

CFLAGS := $(CCXXFLAGS)
CXXFLAGS := $(CCXXFLAGS)
LDFLAGS :=
LIB_LDFLAGS := -lpthread
O_TO_C = $(@:$(BASE_DIR)/%.o=%.c)
O_TO_CPP = $(@:$(BASE_DIR)/%.o=%.cpp)
DEP_DIRS := $(BASE_DEP_DIR)

#
# Object as a dependency. Not directly included when linking.
#
obj-deps :=

#
# Object as a dependency, to be compiled as C++.
#
obj-cpp-y :=

#
# Object as a dependency, to be compiled as C.
#
obj-c-y :=

#
# Object as a dependency, do not compile.
# This may have a custom build rule somewhere.
#
obj-y :=

all: $(TARGET_BIN)

include submodules/Makefile
include src/Makefile

#
# Create dependency directories
#
$(DEP_DIRS):
	$(Q)$(MKDIR) -p $(@)

#
# Dependency graph file generator.
#
DEPFLAGS = -MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)"
DEPFLAGS_EXE = -MT "$@" -MMD -MP -MF "$(@:$(BASE_DIR)/%=$(BASE_DEP_DIR)/%.d)"

#
# Include generated dependency graph files.
#
-include $(obj-cpp-y:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)
-include $(obj-c-y:$(BASE_DIR)/%.o=$(BASE_DEP_DIR)/%.d)

$(obj-c-y): | $(DEP_DIRS)
	$(Q)$(CXX) $(INCLUDE_DIR) $(CFLAGS) $(DEPFLAGS) -o $(@) -c $(O_TO_C);

$(obj-cpp-y): | $(DEP_DIRS)
	$(Q)$(CXX) $(INCLUDE_DIR) $(CFLAGS) $(DEPFLAGS) -o $(@) -c $(O_TO_CPP);

$(TARGET_BIN): $(obj-c-y) $(obj-cpp-y) $(obj-y) $(obj-deps)
	$(CXX) $(CCXXFLAGS) -o $(TARGET_BIN) $(obj-c-y) $(obj-cpp-y) $(obj-y) $(LIB_LDFLAGS);

clean:
	@$(RM) -rfv $(DEP_DIRS) $(TARGET_BIN) $(obj-c-y) $(obj-cpp-y) $(obj-y) $(obj-deps);

.PHONY: all clean

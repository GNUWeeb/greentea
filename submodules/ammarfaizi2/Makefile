#
# SPDX-License-Identifier: GPL-2.0-only
#

SDIR_SUBMODULES__AMMARFAIZI2 := $(BASE_DIR)/submodules/ammarfaizi2

DEP_DIRS += \
	$(BASE_DEP_DIR)/submodules/ammarfaizi2 \
	$(BASE_DEP_DIR)/submodules/ammarfaizi2/cppmysql \
	$(BASE_DEP_DIR)/submodules/ammarfaizi2/workqueue2

obj-cpp-y += \
	$(SDIR_SUBMODULES__AMMARFAIZI2)/cppmysql/MySQL.o \
	$(SDIR_SUBMODULES__AMMARFAIZI2)/workqueue2/WorkQueue.o

INCLUDE_DIR += \
	-I$(SDIR_SUBMODULES__AMMARFAIZI2)

LIB_LDFLAGS += \
	-lmysqlclient

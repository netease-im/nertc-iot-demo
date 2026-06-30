PROJECT ?= nertc_host
PROJECT_DIR ?= $(CURDIR)
SDK_ROOT ?= $(abspath ../bk_aidk)
BUILD_DIR ?= $(PROJECT_DIR)/build_codex

.PHONY: all bk7258 clean

all: bk7258

bk7258:
	$(MAKE) -C $(SDK_ROOT) bk7258 PROJECT=$(PROJECT) PROJECT_DIR=$(PROJECT_DIR) BUILD_DIR=$(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

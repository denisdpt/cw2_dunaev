BUILD ?= build
PARLAY_NUM_THREADS ?= 4
CMAKE_BUILD_TYPE ?= Release

.PHONY: build
build:
	mkdir -p $(BUILD)
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD) -j
	ln -sf $(BUILD)/compile_commands.json compile_commands.json

.PHONY: run
run:
	PARLAY_NUM_THREADS=$(PARLAY_NUM_THREADS) ./$(BUILD)/main

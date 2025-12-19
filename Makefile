BUILD ?= build
CMAKE_BUILD_TYPE ?= Release

PARLAY_NUM_THREADS ?= 4
SIDE ?= 300
RUNS ?= 5
THREADS ?= 4

OMP_PROC_BIND ?= true
OMP_PLACES ?= cores

.PHONY: build
build:
	mkdir -p $(BUILD)
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD) -j
	ln -sf $(BUILD)/compile_commands.json compile_commands.json

.PHONY: run
run:
	OMP_PROC_BIND=$(OMP_PROC_BIND) OMP_PLACES=$(OMP_PLACES) PARLAY_NUM_THREADS=$(PARLAY_NUM_THREADS) \
	./$(BUILD)/main --side $(SIDE) --runs $(RUNS) --threads $(THREADS)

.PHONY: bench
bench: run

.PHONY: testonly
testonly:
	PARLAY_NUM_THREADS=1 ./$(BUILD)/main --skip-bench

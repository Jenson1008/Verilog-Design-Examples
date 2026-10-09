CXX ?= g++
SYSTEMC_HOME ?= /usr
SYSTEMC_INCLUDE ?= $(SYSTEMC_HOME)/include
SYSTEMC_LIBDIR ?= $(SYSTEMC_HOME)/lib

OUT_DIR := $(CURDIR)/out
EXECUTABLE := $(OUT_DIR)/half_adder_sim
BUILD_LOG := $(OUT_DIR)/build.log
SIMULATION_LOG := $(OUT_DIR)/simulation.log
TB_SOURCE := $(CURDIR)/half_adder_sc_tb.cpp
SOURCES := \
	$(TB_SOURCE) \
	$(CURDIR)/DUT/half_adder_sc.h \
	$(CURDIR)/stimulus/half_adder_sc_stimulus.h

.PHONY: all run clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) Makefile
	@mkdir -p "$(OUT_DIR)"
	@"$(CXX)" -std=c++17 -Wall -Wextra -Wpedantic \
		-I"$(SYSTEMC_INCLUDE)" -I"$(CURDIR)" "$(TB_SOURCE)" \
		-L"$(SYSTEMC_LIBDIR)" -Wl,-rpath,"$(SYSTEMC_LIBDIR)" \
		-lsystemc -o "$(EXECUTABLE)" >"$(BUILD_LOG)" 2>&1 || { \
			status=$$?; cat "$(BUILD_LOG)"; exit $$status; \
		}
	@chmod 0755 "$(EXECUTABLE)"
	@chmod 0644 "$(BUILD_LOG)"

run: all
	@cd "$(OUT_DIR)" && ./half_adder_sim >"simulation.log" 2>&1; \
		status=$$?; \
		for output in "$(SIMULATION_LOG)" \
			"$(OUT_DIR)/half_adder.csv" \
			"$(OUT_DIR)/half_adder_wave.vcd"; do \
			if [ -e "$$output" ] && ! chmod 0644 "$$output"; then \
				printf 'ERROR: unable to set output permissions: %s\n' "$$output" >&2; \
				status=1; \
			fi; \
		done; \
		{ \
			printf 'Build log: %s\n' "$(BUILD_LOG)"; \
			printf 'Simulation log: %s\n' "$(SIMULATION_LOG)"; \
			printf 'CSV results: %s\n' "$(OUT_DIR)/half_adder.csv"; \
			printf 'VCD waveform: %s\n' "$(OUT_DIR)/half_adder_wave.vcd"; \
		} | tee -a "$(SIMULATION_LOG)" >/dev/null; \
		cat "$(SIMULATION_LOG)"; \
		exit $$status

clean:
	rm -f -- "$(EXECUTABLE)" "$(BUILD_LOG)" "$(SIMULATION_LOG)" \
		"$(OUT_DIR)/half_adder.csv" \
		"$(OUT_DIR)/half_adder_wave.vcd"

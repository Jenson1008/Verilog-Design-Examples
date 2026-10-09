CXX ?= g++
SYSTEMC_HOME ?= /usr
OUT_DIR ?= out
TARGET := $(OUT_DIR)/cntr_3ud_tb
BUILD_LOG := $(OUT_DIR)/cntr_3ud_build.log
SIMULATION_LOG := $(OUT_DIR)/cntr_3ud.log
WAVE := $(OUT_DIR)/cntr_3ud_wave.vcd

CPPFLAGS += -I"$(SYSTEMC_HOME)/include"
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS += -L"$(SYSTEMC_HOME)/lib" -Wl,-rpath,"$(SYSTEMC_HOME)/lib"
LDLIBS += -lsystemc

.PHONY: all run clean

all: $(TARGET)

$(TARGET): cntr_3ud_tb.cpp DUT/cntr_3ud.h stimulus/cntr_3ud_stimulus.h | $(OUT_DIR)
	@status=0; \
	  $(CXX) $(CPPFLAGS) $(CXXFLAGS) "$<" $(LDFLAGS) $(LDLIBS) -o "$@" >"$(BUILD_LOG)" 2>&1 || status=$$?; \
	  chmod 0644 "$(BUILD_LOG)" || exit 1; \
	  cat "$(BUILD_LOG)"; \
	  if [ "$$status" -eq 0 ]; then chmod 0755 "$@" || exit 1; fi; \
	  if [ "$$status" -ne 0 ]; then \
	    printf 'Build failed with exit code %s; diagnostics retained in %s\n' "$$status" "$(BUILD_LOG)" >&2; \
	    exit "$$status"; \
	  fi

$(OUT_DIR):
	mkdir -p "$@"

run: $(TARGET)
	@status=0; \
	  (cd "$(OUT_DIR)" && ./cntr_3ud_tb) || status=$$?; \
	  if [ -f "$(SIMULATION_LOG)" ]; then chmod 0644 "$(SIMULATION_LOG)" || exit 1; fi; \
	  if [ -f "$(WAVE)" ]; then chmod 0644 "$(WAVE)" || exit 1; fi; \
	  cat "$(SIMULATION_LOG)"; \
	  if [ "$$status" -ne 0 ]; then \
	    printf 'Simulation failed with exit code %s; diagnostics retained in %s\n' "$$status" "$(SIMULATION_LOG)" >&2; \
	    exit "$$status"; \
	  fi; \
	  printf 'Simulation log: %s\n' "$(SIMULATION_LOG)"

clean:
	rm -f "$(TARGET)" "$(BUILD_LOG)" "$(SIMULATION_LOG)" "$(WAVE)"

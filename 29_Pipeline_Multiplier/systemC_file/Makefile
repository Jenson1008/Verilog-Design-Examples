CXX ?= g++
SYSTEMC_INCLUDE ?= /usr/include
SYSTEMC_LIBDIR ?= /usr/lib/$(shell $(CXX) -print-multiarch)
CPPFLAGS ?=
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -lsystemc -pthread

OUT_DIR := out
TB := $(OUT_DIR)/pipeline_sc_tb
BUILD_LOG := $(OUT_DIR)/build.log

SOURCES := pipeline_sc_tb.cpp
HEADERS := DUT/pipeline_sc.h stimulus/pipeline_sc_stimulus.h
RTL_SOURCES := ../pipeline.v ../vdcmul_32b.v ../vdcmul_16b.v \
  ../vdcmul_8b.v ../vdcmul_4b.v ../vdcmul_2b.v ../rca64.v \
  ../rca32b.v ../rca16b.v ../rca8b.v ../rca_4b.v ../half_adder.v
RTL_DIGEST = $(shell cd "$(CURDIR)/.." && \
  sha256sum pipeline.v vdcmul_32b.v vdcmul_16b.v vdcmul_8b.v \
  vdcmul_4b.v vdcmul_2b.v rca64.v rca32b.v rca16b.v rca8b.v \
  rca_4b.v half_adder.v | sha256sum | cut -d' ' -f1)

.PHONY: all run clean

all: $(TB)

$(OUT_DIR):
	mkdir -p "$(OUT_DIR)"

$(TB): $(SOURCES) $(HEADERS) $(RTL_SOURCES) | $(OUT_DIR)
	@set +e; \
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I"$(SYSTEMC_INCLUDE)" \
	  $(SOURCES) -L"$(SYSTEMC_LIBDIR)" \
	  -Wl,-rpath,"$(SYSTEMC_LIBDIR)" $(LDFLAGS) $(LDLIBS) \
	  -o "$(TB)" >"$(BUILD_LOG)" 2>&1; \
	status=$$?; \
	cat "$(BUILD_LOG)"; \
	exit $$status

run: all
	cd "$(CURDIR)/$(OUT_DIR)" && \
	  PIPELINE_RTL_SHA256="$(RTL_DIGEST)" ./pipeline_sc_tb

clean:
	rm -f "$(TB)" "$(BUILD_LOG)" "$(OUT_DIR)/simulation.log" \
	  "$(OUT_DIR)/checks.csv" "$(OUT_DIR)/wave.vcd"

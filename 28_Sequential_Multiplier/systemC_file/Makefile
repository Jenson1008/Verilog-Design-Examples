CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
SYSTEMC_CXXFLAGS ?= -I/usr/include
SYSTEMC_LDFLAGS ?= -L/usr/lib/x86_64-linux-gnu -Wl,-rpath,/usr/lib/x86_64-linux-gnu
SYSTEMC_LIBS ?= -lsystemc -pthread
OUT := out
TARGET := $(OUT)/seqmult_sc_tb
SOURCES := seqmult_sc_tb.cpp DUT/seqmult_sc.h stimulus/seqmult_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CXXFLAGS) $(SYSTEMC_CXXFLAGS) -I. seqmult_sc_tb.cpp $(SYSTEMC_LDFLAGS) $(SYSTEMC_LIBS) -o "$(TARGET)" >"$(OUT)/build.log" 2>&1 || { code=$$?; cat "$(OUT)/build.log"; exit $$code; }
	@cat "$(OUT)/build.log"
run: all
	cd "$(OUT)" && ./seqmult_sc_tb >simulation.log 2>&1; code=$$?; cat simulation.log; exit $$code
clean:
	rm -f "$(OUT)/seqmult_sc_tb" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/checks.csv" "$(OUT)/wave.vcd"

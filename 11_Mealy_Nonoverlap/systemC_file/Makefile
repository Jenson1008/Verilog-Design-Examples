CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -I.
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/mealy_nonol_sim
SOURCES := mealy_nonol_sc_tb.cpp DUT/mealy_nonol_sc.h stimulus/mealy_nonol_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) mealy_nonol_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	bash -o pipefail -c 'cd "$(OUT)" && ./mealy_nonol_sim | tee simulation.log'
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/mealy_nonol.csv" "$(OUT)/mealy_nonol_wave.vcd"

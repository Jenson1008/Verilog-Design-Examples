CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -IDUT -Istimulus
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/mux_case_sim
SOURCES := mux_case_sc_tb.cpp DUT/mux_case_sc.h stimulus/mux_case_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) mux_case_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	cd "$(OUT)" && ./mux_case_sim
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/mux_case.csv" "$(OUT)/mux_case_wave.vcd"

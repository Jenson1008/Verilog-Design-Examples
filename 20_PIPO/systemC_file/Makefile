CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -I.
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/pipo_shift_sim
SOURCES := pipo_shift_sc_tb.cpp DUT/pipo_shift_sc.h stimulus/pipo_shift_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) pipo_shift_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	bash -o pipefail -c 'cd "$(OUT)" && ./pipo_shift_sim | tee simulation.log'
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/pipo_shift.csv" "$(OUT)/pipo_shift_wave.vcd"

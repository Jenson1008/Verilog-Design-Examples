CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -IDUT -Istimulus
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/half_adder_sim
SOURCES := half_adder_sc_tb.cpp DUT/half_adder_sc.h stimulus/half_adder_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) half_adder_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	cd "$(OUT)" && ./half_adder_sim
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/half_adder.csv" "$(OUT)/half_adder_wave.vcd"

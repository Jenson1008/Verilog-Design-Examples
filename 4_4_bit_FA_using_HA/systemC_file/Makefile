CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -IDUT -Istimulus
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/full_add_4bit_sim
SOURCES := full_add_4bit_sc_tb.cpp DUT/full_add_4bit_sc.h stimulus/full_add_4bit_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) full_add_4bit_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	cd "$(OUT)" && ./full_add_4bit_sim
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/full_add_4bit.csv" "$(OUT)/full_add_4bit_wave.vcd"

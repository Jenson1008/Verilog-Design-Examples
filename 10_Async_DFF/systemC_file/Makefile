CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -IDUT -Istimulus
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/dff_async_sim
SOURCES := dff_async_sc_tb.cpp DUT/dff_async_sc.h stimulus/dff_async_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) dff_async_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	cd "$(OUT)" && ./dff_async_sim
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/dff_async.csv" "$(OUT)/dff_async_wave.vcd"

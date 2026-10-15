CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -I.
LDLIBS ?= -lsystemc
OUT := out
TARGET := $(OUT)/no_1s_sim
SOURCES := no_1s_sc_tb.cpp DUT/no_1s_sc.h stimulus/no_1s_sc_stimulus.h
.PHONY: all run clean
all: $(TARGET)
$(TARGET): $(SOURCES)
	mkdir -p "$(OUT)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) no_1s_sc_tb.cpp $(LDLIBS) -o "$@" > "$(OUT)/build.log" 2>&1 || { status=$$?; cat "$(OUT)/build.log"; exit $$status; }
run: $(TARGET)
	bash -o pipefail -c 'cd "$(OUT)" && ./no_1s_sim | tee simulation.log'
clean:
	rm -f "$(TARGET)" "$(OUT)/build.log" "$(OUT)/simulation.log" "$(OUT)/no_1s.csv" "$(OUT)/no_1s_wave.vcd"

#!/usr/bin/make -f
NAME = BassEQ8
FILES_DSP = BassEQ8_dsp.cpp
FILES_UI  = BassEQ8_ui.cpp
DPF_PATH ?= ../DPF
CXXFLAGS += -I..
include $(DPF_PATH)/Makefile.plugins.mk
TARGETS += vst3
all: $(TARGETS)

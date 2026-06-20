#!/usr/bin/make -f
NAME = EQ5
FILES_DSP = EQ5_dsp.cpp
FILES_UI  = EQ5_ui.cpp
DPF_PATH ?= ../DPF
CXXFLAGS += -I..
include $(DPF_PATH)/Makefile.plugins.mk
TARGETS += vst3
all: $(TARGETS)

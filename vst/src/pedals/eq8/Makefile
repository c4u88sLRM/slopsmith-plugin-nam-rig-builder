#!/usr/bin/make -f
NAME = EQ8
FILES_DSP = EQ8_dsp.cpp
FILES_UI  = EQ8_ui.cpp
DPF_PATH ?= ../DPF
CXXFLAGS += -I../..
include $(DPF_PATH)/Makefile.plugins.mk
TARGETS += vst3
all: $(TARGETS)

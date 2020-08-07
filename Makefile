#================================================================
#   Copyright (C) 2020 Techyauld Ltd. All rights reserved.
#
#   File Name:Makefile
#   Author:Yang Jian <jyang@techyauld.com>
#   Create Date:2020-07-15
#   Description:
#
#================================================================
ifdef CROSS_COMPILE
	CROSS_TOOL ?= arm-cortex_a8-linux-gnueabihf-
endif

ifdef DEBUG
	IFDEBUG ?= -DDEBUG
endif

CC=$(CROSS_TOOL)gcc
CFLAGS=$(IFDEBUG) -g -Wall
LFLAGS ?= -lpthread

SOURCE_DIR=.
SOURCE_FILE=$(wildcard $(SOURCE_DIR)/*.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE_FILE))
PROGS=$(patsubst %.c,%,$(SOURCE_FILE))

.PHONY:all clean

all:$(PROGS)

$(PROGS):%:%.c
	$(CC) -o $@ $(CFLAGS) $< $(LFLAGS)

clean:
	@ rm -f $(PROGS)

#=========================end of this file=======================

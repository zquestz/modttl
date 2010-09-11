# Makefile for modttl
# (C) quest

AR = ar
GFLAGS =
GET = get
ASFLAGS =
MAS = mas
AS = as
FC = f77
CFLAGS = 
CC = cc
CPPFLAGS = -O3
# CPPFLAGS = -O3 -arch ppc -arch ppc64 -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk
CPP = c++
LDFLAGS =
LD = ld
LFLAGS =
LEX = lex
YFLAGS =
YACC = yacc
LOADLIBS =
MAKE = make
MAKEARGS = 'SHELL=/bin/sh'
SHELL = /bin/sh
MAKEFLAGS = b

SRCS = modttl.cpp
ifeq ($(shell uname -s),FreeBSD)
LIBS = -pthread
else
LIBS = -lpthread
endif

APPNAME = modttl

modttl: ${SRCS}
	${CPP} -o ${APPNAME} ${CPPFLAGS} ${SRCS} ${LIBS}

clean:
	cd ..; rm ${APPNAME}

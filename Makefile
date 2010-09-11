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
CPPFLAGS =
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

APPNAME = modttl

all:
	cd src; make ${APPNAME}; mv ${APPNAME} ..; cd ..

install: all
	install -d /usr/local/sbin
	install -c ${APPNAME} /usr/local/sbin/modttl
	install -c modttl-startup /usr/local/sbin/modttl-startup
clean:
	cd src; make clean

uninstall:
	rm /usr/local/sbin/modttl
	rm /usr/local/sbin/modttl-startup

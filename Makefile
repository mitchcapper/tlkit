#
#    This file is part of tlkit
#    Copyright (C) 2015, Alejandro Liu
#
#    tlkit is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    tlkit is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, see <http://www.gnu.org/licenses>
#
#_begin := $(shell ./scripts/init.sh)

PKGNAME = tlkit
TARGET = #arm-mv5sft-linux-gnueabi
CC = $(TARGET)gcc
LD = $(TARGET)gcc
AR = $(TARGET)ar
VERSION = 0.1.0
CVERSION = tlk_$(VERSION)
BUILD_NUM = 0

PEDANTIC=-Wall -Wextra -std=gnu99 -pedantic

GOPTZ = -Og
OPTFLAGS = -g -D_DEBUG $(GOPTZ) # Debug build
#OPTIMIZ = -O3 # Prod build

CFLAGS = $(OPTFLAGS) $(PEDANTIC) $(INCDIRS) $(XCFLAGS) -DVERSION=\"$(CVERSION)\"
#-O2 -I$(UTHASH_DIR)/src -I$(GDBM_LIBDIR)/src #-DHASH_TYPE=SHA256
EXES = genrand lomount md5tee mkproto qt-faststart rvc w4d
SRCS = Makefile *.c *.h *.txt *.md LICENSE
PREFIX = /usr
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man


all: $(EXES)

tar:
	make $(PKGNAME).spec
	tmp=$$(mktemp -d -p .) ; \
	mkdir $$tmp/$(PKGNAME)-$(VERSION) ; \
	mv $(PKGNAME).spec $$tmp/$(PKGNAME)-$(VERSION) ; \
	cp $(SRCS) $$tmp/$(PKGNAME)-$(VERSION) ; \
	tar zcf $(PKGNAME)-$(VERSION).tar.gz -C $$tmp $(PKGNAME)-$(VERSION) ; \
	rm -rf $$tmp

$(PKGNAME).spec: $(PKGNAME).spec.in
	sed \
	   -e 's!@VERSION@!$(VERSION)!' \
	   -e 's!@BUILD_NUM@!$(BUILD_NUM)!' $< > $@

install:
	mkdir -p $(DESTDIR)$(BINDIR) ; \
	for f in $(EXES) ; do \
	  install -D -m 755 $$f $(DESTDIR)$(BINDIR)/$$f ; \
	done

docs:


clean:
	rm -f $(EXES) tlkit.spec

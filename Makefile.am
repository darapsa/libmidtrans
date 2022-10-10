ACLOCAL_AMFLAGS = -I m4
lib_LTLIBRARIES = libmidtrans.la
libmidtrans_la_SOURCES = midtrans.c
libmidtrans_la_CPPFLAGS = $(DEPS_CFLAGS)
libmidtrans_la_LDFLAGS = $(DEPS_LIBS)
if IOS
libmidtrans_la_LDFLAGS += -static
endif
include_HEADERS = midtrans.h

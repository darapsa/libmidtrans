lib_LTLIBRARIES = libmidtrans.la
libmidtrans_la_SOURCES = midtrans.c
libmidtrans_la_CPPFLAGS = $(JSON_CFLAGS)
if IOS
libmidtrans_la_LDFLAGS = -static
endif
if WASM
libmidtrans_la_LDFLAGS = -static
else
libmidtrans_la_CPPFLAGS += $(CURL_CFLAGS)
endif
include_HEADERS = midtrans.h

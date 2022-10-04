#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#else
#include <curl/curl.h>
#endif
#include <json.h>
#include "midtrans.h"

void midtrans_init(const char *certificate)
{
#ifndef __EMSCRIPTEN__
	curl_global_init(CURL_GLOBAL_SSL);
#endif
}

void midtrans_cleanup()
{
#ifndef __EMSCRIPTEN__
	curl_global_cleanup();
#endif
}

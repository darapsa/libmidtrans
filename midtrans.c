#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#else
#include <curl/curl.h>
#endif
#include <json.h>
#include "midtrans.h"

#ifndef __EMSCRIPTEN__
static char *cainfo = NULL;
#endif

void midtrans_init(const char *certificate)
{
#ifndef __EMSCRIPTEN__
	curl_global_init(CURL_GLOBAL_SSL);
	if (certificate) {
		cainfo = malloc(strlen(certificate) + 1);
		strcpy(cainfo, certificate);
	}
#endif
}

void midtrans_cleanup()
{
#ifndef __EMSCRIPTEN__
	if (cainfo)
		free(cainfo);
	curl_global_cleanup();
#endif
}

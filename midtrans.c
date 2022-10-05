#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <json.h>
#include "midtrans.h"

#define ORDER_ID "SANDBOX-G710367688-806"

static _Bool sandbox;
static char *base_url;
static CURL *curl;

void midtrans_init(_Bool production, const char *api_key, const char *cainfo)
{
	sandbox = !production;
	static const char *url_tmpl = "https://api.%smidtrans.com/v2/";
	static const char *infix = "sandbox.";
	base_url = malloc(strlen(url_tmpl) - strlen ("%s")
			+ sandbox * strlen(infix) + 1);
	sprintf(base_url, url_tmpl, production ? "" : infix);
	curl_global_init(CURL_GLOBAL_SSL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	struct curl_slist *list
		= curl_slist_append(NULL, "Accept: application/json");
	list = curl_slist_append(list, "Content-Type: application/json");
	static const char *hdr_tmpl = "Authorization: %s";
	char auth[strlen(hdr_tmpl) - strlen("%s") + strlen(api_key) + 1];
	sprintf(auth, hdr_tmpl, api_key);
	list = curl_slist_append(list, auth);
	if (cainfo)
		curl_easy_setopt(curl, CURLOPT_CAINFO, cainfo);
}

void midtrans_status(const char *order_id)
{
	static const char *tmpl = "%s%s/status";
	char url[strlen(tmpl) - strlen("%s") * 2 + strlen(base_url)
		+ (sandbox ? strlen(ORDER_ID) : strlen(order_id)) + 1];
	sprintf(url, tmpl, base_url, sandbox ? ORDER_ID : order_id);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_perform(curl);
}

void midtrans_cleanup()
{
	free(base_url);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

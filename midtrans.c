#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <pthread.h>
#include <curl/curl.h>
#include <json.h>
#include "midtrans.h"

#define ORDER_ID "SANDBOX-G710367688-806"

static _Bool production = 0;
static char *base_url;
static CURL *curl;
static struct curl_slist *slist;
static pthread_t *threads;
static size_t num_threads = 0;

static size_t append(char *data, size_t size, size_t nmemb, char **json)
{
	size_t realsize = size * nmemb;
	size_t json_len = *json ? strlen(*json) : 0;
	*json = realloc(*json, json_len + realsize + 1);
	strlcpy(&(*json)[json_len], data, realsize + 1);
	return realsize;
}

void midtrans_init(const char *api_key, const char *cainfo)
{
	static const char *prefix = "SB-";
	if (strncmp(api_key, prefix, strlen(prefix)))
		production = 1;
	static const char *url_tmpl = "https://api.%smidtrans.com/v2/";
	static const char *infix = "sandbox.";
	base_url = malloc(strlen(url_tmpl) - strlen ("%s")
			+ !production * strlen(infix) + 1);
	sprintf(base_url, url_tmpl, production ? "" : infix);

	curl_global_init(CURL_GLOBAL_SSL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
#ifdef DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

	static const char *basic_tmpl = "%s:";
	const size_t basic_len = strlen(basic_tmpl) - strlen("%s")
		+ strlen(api_key);
	char basic[basic_len + 1];
	sprintf(basic, basic_tmpl, api_key);

	BIO *b64 = BIO_new(BIO_f_base64());
	BIO *mem = BIO_new(BIO_s_mem());
	BIO_push(b64, mem);
	BIO_write(b64, basic, basic_len);
	BIO_flush(b64);
	char *pp;
	long base64_len = BIO_get_mem_data(b64, &pp) - 1;
	char base64[base64_len + 1];
	strlcpy(base64, pp, base64_len + 1);
	BIO_free_all(b64);

	static const char *hdr_tmpl = "Authorization: Basic %s";
	char auth[strlen(hdr_tmpl) - strlen("%s") + base64_len + 1];
	sprintf(auth, hdr_tmpl, base64);
	slist = curl_slist_append(NULL, auth);
	slist = curl_slist_append(slist, "Accept: application/json");
	slist = curl_slist_append(slist, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

	if (cainfo)
		curl_easy_setopt(curl, CURLOPT_CAINFO, cainfo);
}

static void *request(void *arg)
{
	char *json = NULL;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);
	curl_easy_perform(curl);
#ifdef DEBUG
	printf("%s\n", json);
#endif
	free(json);
	return NULL;
}

void midtrans_status(const char *order_id)
{
	static const char *tmpl = "%s%s/status";
	char url[strlen(tmpl) - strlen("%s") * 2 + strlen(base_url)
		+ (production ? strlen(order_id) : strlen(ORDER_ID)) + 1];
	sprintf(url, tmpl, base_url, production ? order_id : ORDER_ID);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	threads = realloc(threads, ++num_threads * sizeof(pthread_t));
	pthread_create(&threads[num_threads - 1], NULL, request, NULL);
}

void midtrans_cleanup()
{
	for (size_t i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);
	free(base_url);
	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <curl/curl.h>
#include <json.h>
#include "midtrans.h"

#define ORDER_ID "SANDBOX-G710367688-806"

static _Bool production = 0;
static char *base_url;
static CURL *curl;
static struct curl_slist *slist;

struct response {
	size_t size;
	char *data;
};

static size_t append(char *data, size_t size, size_t nmemb,
		struct response *res)
{
	size_t realsize = size * nmemb;
	res->data = realloc(res->data, res->size + realsize + 1);
	strncpy(&res->data[res->size], data, realsize);
	res->size += realsize;
	res->data[res->size] = '\0';
	return realsize;
}

void midtrans_init(const char *api_key, char *pem)
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
	strncpy(base64, pp, base64_len);
	base64[base64_len] = '\0';
	BIO_free_all(b64);

	static const char *hdr_tmpl = "Authorization: Basic %s";
	char auth[strlen(hdr_tmpl) - strlen("%s") + base64_len + 1];
	sprintf(auth, hdr_tmpl, base64);
	slist = curl_slist_append(NULL, auth);
	slist = curl_slist_append(slist, "Accept: application/json");
	slist = curl_slist_append(slist, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

	if (pem)
		curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB,
				(&(struct curl_blob){
					.data = pem,
					.len = strlen(pem),
					.flags = CURL_BLOB_COPY
				}));
}

static void *request(void *arg)
{
	struct response res = { 0, NULL };
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_perform(curl);
#ifdef DEBUG
	printf("%s\n", res.data);
#endif
	free(res.data);
	return NULL;
}

void midtrans_status(const char *order_id)
{
	static const char *tmpl = "%s%s/status";
	char url[strlen(tmpl) - strlen("%s") * 2 + strlen(base_url)
		+ (production ? strlen(order_id) : strlen(ORDER_ID)) + 1];
	sprintf(url, tmpl, base_url, production ? order_id : ORDER_ID);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	request(NULL);
}

void midtrans_charge_banktransfer(struct midtrans_banktransfer *banktransfer,
		struct midtrans_transaction *transaction, char *custom_fields[])
{
	static const char *url_tmpl = "%scharge";
	char url[strlen(url_tmpl) - strlen("%s") + strlen(base_url) + 1];
	sprintf(url, url_tmpl, base_url);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	static const char *va_number_tmpl = ",\n"
		"\t\t\"va_number\": \"%s\"";
	size_t va_number_len = 0;
	char *va_number = NULL;
	if (banktransfer->va_number) {
		va_number_len = strlen(va_number_tmpl) - strlen("%s")
			+ strlen(banktransfer->va_number);
		va_number = malloc(va_number_len + 1);
		sprintf(va_number, va_number_tmpl, banktransfer->va_number);
	}

	static const char *payment_tmpl = "bank_transfer\",\n"
		"\t\"bank_transfer\": {\n"
		"\t\t\"bank\": \"%s\"%s";
	const size_t payment_len = strlen(payment_tmpl) - strlen("%s") * 2
		+ va_number_len + strlen(banktransfer->bank);
	char *payment = malloc(payment_len + 1);
	sprintf(payment, payment_tmpl, banktransfer->bank,
			va_number_len ? va_number : "");

	size_t i = 0;
	size_t fields_len = 0;
	char *fields = NULL;
	static const char *field_tmpl = "\n\t\"%s\": \"%s\",";
	const size_t field_static_len = strlen(field_tmpl) - strlen("%s") * 2;
	while (i < 5 && custom_fields && custom_fields[i]
			&& custom_fields[i + 1]) {
		size_t field_len = field_static_len + strlen(custom_fields[i])
			+ strlen(custom_fields[i + 1]);
		char field[field_len + 1];
		sprintf(field, field_tmpl, custom_fields[i],
				custom_fields[i + 1]);
		fields = realloc(fields, fields_len + field_len + 1);
		strcpy(&fields[fields_len], field);
		fields_len += field_len;
		i += 2;
	}

	static const char *post_tmpl =
		"{\n"
		"\t\"payment_type\": \"%s"
		"\n\t},"
		"%s\n"
		"\t\"transaction_details\": {\n"
		"\t\t\"order_id\": \"%s\",\n"
		"\t\t\"gross_amount\": %ld\n"
		"\t}\n"
		"}";
	long gross_amount = transaction->gross_amount;
	size_t gross_amount_len = 1;
	while ((gross_amount /= 10))
		gross_amount_len++;
	char post[strlen(post_tmpl) - strlen("%s") * 3 - strlen("%ld")
		+ payment_len + fields_len + strlen(transaction->order_id)
		+ gross_amount_len + 1];
	sprintf(post, post_tmpl, payment, fields_len ? fields : "",
			transaction->order_id, transaction->gross_amount);
	free(payment);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);

	request(NULL);
}

void midtrans_cleanup()
{
	free(base_url);
	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

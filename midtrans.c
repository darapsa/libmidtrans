#include <stdio.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <curl/curl.h>
#include <json.h>
#include "midtrans.h"

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

struct midtrans_banktransfer midtrans_banktransfer_new(char *bank)
{
	return (struct midtrans_banktransfer){ bank, NULL, NULL, NULL };
}

struct midtrans_echannel midtrans_echannel_new(char *bill_info1,
		char *bill_info2)
{
	return (struct midtrans_echannel){ bill_info1, bill_info2 };
}

struct midtrans_transaction midtrans_transaction_new(char *order_id,
		long gross_amount)
{
	return (struct midtrans_transaction){ order_id, gross_amount };
}

char *midtrans_charge_banktransfer(struct midtrans_banktransfer banktransfer,
		struct midtrans_transaction transaction
		/*, char *custom_fields[]*/)
{
	static const char *url_tmpl = "%scharge";
	char url[strlen(url_tmpl) - strlen("%s") + strlen(base_url) + 1];
	sprintf(url, url_tmpl, base_url);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	static const char *va_number_tmpl = ",\n"
		"\t\t\"va_number\": \"%s\"";
	size_t va_number_len = 0;
	char *va_number = NULL;
	if (banktransfer.va_number) {
		va_number_len = strlen(va_number_tmpl) - strlen("%s")
			+ strlen(banktransfer.va_number);
		va_number = malloc(va_number_len + 1);
		sprintf(va_number, va_number_tmpl, banktransfer.va_number);
	}

	static const char *payment_tmpl = "bank_transfer\",\n"
		"\t\"bank_transfer\": {\n"
		"\t\t\"bank\": \"%s\"%s";
	const size_t payment_len = strlen(payment_tmpl) - strlen("%s") * 2
		+ va_number_len + strlen(banktransfer.bank);
	char *payment = malloc(payment_len + 1);
	sprintf(payment, payment_tmpl, banktransfer.bank,
			va_number_len ? va_number : "");

	size_t i = 0;
	size_t fields_len = 0;
	char *fields = NULL;
	static const char *field_tmpl = "\n\t\"%s\": \"%s\",";
	const size_t field_static_len = strlen(field_tmpl) - strlen("%s") * 2;
	/*
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
	*/

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
	long gross_amount = transaction.gross_amount;
	size_t gross_amount_len = 1;
	while ((gross_amount /= 10))
		gross_amount_len++;
	char post[strlen(post_tmpl) - strlen("%s") * 3 - strlen("%ld")
		+ payment_len + fields_len + strlen(transaction.order_id)
		+ gross_amount_len + 1];
	sprintf(post, post_tmpl, payment, fields_len ? fields : "",
			transaction.order_id, transaction.gross_amount);
	free(payment);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);

	struct response res = { 0, NULL };
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_perform(curl);

	json_tokener *tokener = json_tokener_new();
	json_object *status = json_tokener_parse_ex(tokener, res.data,
			res.size);
	free(res.data);

	json_object *number = NULL;
	if (strcmp(banktransfer.bank, "permata")) {
		json_object *va_numbers = NULL;
		json_object_object_get_ex(status, "va_numbers", &va_numbers);
		json_object *object = json_object_array_get_idx(va_numbers, 0);
		json_object_object_get_ex(object, "va_number", &number);
	} else
		json_object_object_get_ex(status, "permata_va_number", &number);
	const char *string = json_object_get_string(number);
	char *virtualaccount_number = malloc(strlen(string) + 1);
	strcpy(virtualaccount_number, string);
	json_tokener_free(tokener);

	return virtualaccount_number;
}

char *midtrans_charge_echannel(struct midtrans_echannel echannel,
		struct midtrans_transaction transaction)
{
	static const char *url_tmpl = "%scharge";
	char url[strlen(url_tmpl) - strlen("%s") + strlen(base_url) + 1];
	sprintf(url, url_tmpl, base_url);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	static const char *payment_tmpl = "echannel\",\n"
		"\t\"echannel\": {\n"
		"\t\t\"bill_info1\": \"%s\","
		"\t\t\"bill_info2\": \"%s\"";
	const size_t payment_len = strlen(payment_tmpl) - strlen("%s") * 2
		+ strlen(echannel.bill_info1) + strlen(echannel.bill_info2);
	char *payment = malloc(payment_len + 1);
	sprintf(payment, payment_tmpl, echannel.bill_info1,
			echannel.bill_info2);

	static const char *post_tmpl =
		"{\n"
		"\t\"payment_type\": \"%s\n"
		"\t},\n"
		"\t\"transaction_details\": {\n"
		"\t\t\"order_id\": \"%s\",\n"
		"\t\t\"gross_amount\": %ld\n"
		"\t}\n"
		"}";
	long gross_amount = transaction.gross_amount;
	size_t gross_amount_len = 1;
	while ((gross_amount /= 10))
		gross_amount_len++;
	char post[strlen(post_tmpl) - strlen("%s") * 3 - strlen("%ld")
		+ payment_len + strlen(transaction.order_id) + gross_amount_len
		+ 1];
	sprintf(post, post_tmpl, payment, transaction.order_id,
			transaction.gross_amount);
	free(payment);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);

	struct response res = { 0, NULL };
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_perform(curl);

	json_tokener *tokener = json_tokener_new();
	json_object *status = json_tokener_parse_ex(tokener, res.data,
			res.size);
	free(res.data);

	json_object *bill_key = NULL;
	json_object_object_get_ex(status, "bill_key", &bill_key);
	const char *string = json_object_get_string(bill_key);
	char *key = malloc(strlen(string) + 1);
	strcpy(key, string);
	json_tokener_free(tokener);

	return key;
}

char *midtrans_status(const char *order_id)
{
	static const char *tmpl = "%s%s/status";
	char url[strlen(tmpl) - strlen("%s") * 2 + strlen(base_url)
		+ strlen(order_id) + 1];
	sprintf(url, tmpl, base_url, order_id);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

	struct response res = { 0, NULL };
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
	curl_easy_perform(curl);

	json_tokener *tokener = json_tokener_new();
	json_object *object = json_tokener_parse_ex(tokener, res.data,
			res.size);
	free(res.data);

	json_object *transaction_status = NULL;
	json_object_object_get_ex(object, "transaction_status",
			&transaction_status);
	const char *string = json_object_get_string(transaction_status);
	char *status = malloc(strlen(string) + 1);
	strcpy(status, string);
	json_tokener_free(tokener);

	return status;
}

void midtrans_cleanup()
{
	free(base_url);
	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

enum midtrans_transaction_status midtrans_notification_transaction(char *post,
		const char *server_key,
		struct midtrans_transaction *transaction)
{
	json_tokener *tokener = json_tokener_new();
	json_object *object = json_tokener_parse_ex(tokener, post,
			strlen(post));

	enum midtrans_transaction_status status = 0;
	const char *status_code = NULL;
	const char *signature_key = NULL;
	const char *order_id = NULL;
	const char *gross_amount = NULL;

	struct json_object_iterator iter = json_object_iter_begin(object);
	struct json_object_iterator iter_end = json_object_iter_end(object);
	while (!json_object_iter_equal(&iter, &iter_end)) {
		const char *val = json_object_get_string(
				json_object_iter_peek_value(&iter));
		if (!strcmp(json_object_iter_peek_name(&iter), "status_code"))
			status_code = val;
		else if (!strcmp(json_object_iter_peek_name(&iter),
					"signature_key"))
			signature_key = val;
		else if (!strcmp(json_object_iter_peek_name(&iter),
					"order_id"))
			order_id = val;
		else if (!strcmp(json_object_iter_peek_name(&iter),
					"gross_amount"))
			gross_amount = val;
		json_object_iter_next(&iter);
	}
	json_tokener_free(tokener);

	size_t order_id_len = strlen(order_id);
	size_t signature_fields_len = order_id_len + strlen(status_code)
		+ strlen(gross_amount) + strlen(server_key);
	char signature_fields[signature_fields_len + 1];
	sprintf(signature_fields, "%s%s%s%s", transaction->order_id,
			status_code, gross_amount, server_key);

	BIO *bio = BIO_new(BIO_s_null());
	BIO *mdtmp = BIO_new(BIO_f_md());
	BIO_set_md(mdtmp, EVP_sha512());
	bio = BIO_push(mdtmp, bio);
	BIO_write(bio, signature_fields, signature_fields_len);
	BIO_flush(bio);
	char *pp;
	long hash_len = BIO_get_mem_data(bio, &pp) - 1;
	char hash[hash_len + 1];
	strncpy(hash, pp, hash_len);
	hash[hash_len] = '\0';
	BIO_free_all(bio);

	if (strcmp(signature_key, hash))
		return 0;

	transaction->order_id = malloc(order_id_len + 1);
	strcpy(transaction->order_id, order_id);
	transaction->gross_amount = strtol(gross_amount, NULL, 10);

	return strtol(status_code, NULL, 10);
}

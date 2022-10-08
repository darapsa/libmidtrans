#ifndef MIDTRANS_H
#define MIDTRANS_H

struct midtrans_transaction {
	char *order_id;
	long gross_amount;
};

enum midtrans_payment {
	MIDTRANS_CREDITCARD = 0,
	MIDTRANS_BANKTRANSFER,
	MIDTRANS_BCA_KLIKBCA,
	MIDTRANS_BCA_KLIKPAY,
	MIDTRANS_BRI_EPAY,
	MIDTRANS_CIMB_CLICKS,
	MIDTRANS_DANAMON_ONLINE,
	MIDTRANS_UOB_EZPAY,
	MIDTRANS_QRIS,
	MIDTRANS_GOPAY,
	MIDTRANS_SHOPEEPAY,
	MIDTRANS_CSTORE,
	MIDTRANS_AKULAKU,
	MIDTRANS_KREDIVO
};

struct midtrans_banktransfer {
	char *bank;
	char *va_number;
	char *bca;
	char *permata;
};

#ifdef __cplusplus
extern "C" {
#endif

void midtrans_init(const char *api_key, const char *cainfo);
void midtrans_status(const char *order_id);
void midtrans_charge(enum midtrans_payment type, void *payment,
		struct midtrans_transaction *transaction,
		char *custom_fields[]);
void midtrans_cleanup();

#ifdef __cplusplus
}
#endif

#endif

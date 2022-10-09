#ifndef MIDTRANS_H
#define MIDTRANS_H

struct midtrans_transaction {
	char *order_id;
	long gross_amount;
};

struct midtrans_banktransfer {
	char *bank;
	char *va_number;
	char *bca;
	char *permata;
};

#define midtrans_charge(x, y, z) _Generic((x),\
		struct midtrans_banktransfer *:\
		midtrans_charge_banktransfer(x, y, z)\
		)

#ifdef __cplusplus
extern "C" {
#endif

void midtrans_init(const char *api_key, const char *cainfo);
void midtrans_status(const char *order_id);
void midtrans_charge_banktransfer(struct midtrans_banktransfer *banktransfer,
		struct midtrans_transaction *transaction,
		char *custom_fields[]);
void midtrans_cleanup();

#ifdef __cplusplus
}
#endif

#endif

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

struct midtrans_echannel {
	char *bill_info1;
	char *bill_info2;
};

#define midtrans_charge(x, y) _Generic((x),\
		struct midtrans_banktransfer: midtrans_charge_banktransfer,\
		struct midtrans_echannel: midtrans_charge_echannel\
		)(x, y)

#ifdef __cplusplus
extern "C" {
#endif

void midtrans_init(const char *api_key, char *pem);
struct midtrans_banktransfer midtrans_banktransfer_new(char *bank);
struct midtrans_echannel midtrans_echannel_new(char *bill_info1,
		char *bill_info2);
struct midtrans_transaction midtrans_transaction_new(char *order_id,
		long gross_amount);
char *midtrans_charge_banktransfer(struct midtrans_banktransfer banktransfer,
		struct midtrans_transaction transaction
		/*, char *custom_fields[]*/);
char *midtrans_charge_echannel(struct midtrans_echannel echannel,
		struct midtrans_transaction transaction);
char *midtrans_status(const char *order_id);
void midtrans_cleanup();

#ifdef __cplusplus
}
#endif

#endif

#ifndef MIDTRANS_H
#define MIDTRANS_H

#ifdef __cplusplus
extern "C" {
#endif

void midtrans_init(const char *api_key, const char *cainfo);
void midtrans_status(const char *order_id);
void midtrans_cleanup();

#ifdef __cplusplus
}
#endif

#endif

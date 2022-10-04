#ifndef MIDTRANS_H
#define MIDTRANS_H

#ifdef __cplusplus
extern "C" {
#endif

void midtrans_init(const char *certificate);
void midtrans_cleanup();

#ifdef __cplusplus
}
#endif

#endif

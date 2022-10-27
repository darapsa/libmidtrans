import 'dart:ffi';
import 'dart:io';

import 'package:ffi/ffi.dart';
import 'package:path/path.dart' as path;

class MidtransTransaction extends Struct {
	Pointer<Utf8> order_id;

	@Long()
	int gross_amount;
}

class MidtransBanktransfer extends Struct {
	Pointer<Utf8> bank;
	Pointer<Utf8> va_number;
	Pointer<Utf8> bca;
	Pointer<Utf8> permata;
}

class Midtrans {
	DynamicLibrary dylib;

	Midtrans(String apiKey, String pem) {
		dylib = Platform.isAndroid
			? DynamicLibrary.open('libmidtrans.so')
			: DynamicLibrary.process();

		final apiKeyUtf8 = apiKey.toNativeUtf8();
		final pemUtf8 = pem.toNativeUtf8();
		final midtrans_init = dylib.lookupFunction
			<Void Function(Pointer<Utf8>, Pointer<Utf8>),
			void Function(Pointer<Utf8>, Pointer<Utf8>)>
				('midtrans_init');
		midtrans_init(apiKeyUtf8, pemUtf8);
		calloc.free(apiKeyUtf8);
		calloc.free(pemUtf8);
	}

	void chargeBanktransfer(MidtransBanktransfer payment,
			MidtransTransaction transaction,
			Array<Pointer<Utf8>> customFields) {
		final midtrans_charge = dylib.lookupFunction
			<Void Function(MidtransBanktransfer,
					MidtransTransaction,
					Array<Pointer<Utf8>>),
		void Function(MidtransBanktransfer, MidtransTransaction,
				Array<Pointer<Utf8>>)>
			('midtrans_charge_banktransfer');
		midtrans_charge(payment, transaction, customFields);
		calloc.free(payment.bank);
		if (payment.va_number != null) {
			calloc.free(payment.va_number);
		}
		if (payment.bca != null) {
			calloc.free(payment.bca);
		}
		if (payment.permata != null) {
			calloc.free(payment.permata);
		}
		calloc.free(transaction.order_id);
		for (var i = 0; i < 6; i++) {
			if (customFields[i] != null) {
				calloc.free(customFields[i]);
			}
		}
	}

	void cleanup() {
		final midtrans_cleanup = dylib.lookupFunction<Void Function(),
		      void Function()>('midtrans_cleanup');
		midtrans_cleanup();
	}
}

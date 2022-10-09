import 'dart:ffi';
import 'dart:io';

import 'package:ffi/ffi.dart';
import 'package:path/path.dart' as path;

class MidtransTransaction extends Struct {
	external Pointer<Utf8> order_id;

	@Long()
	external long gross_amount;
}

class MidtransBanktransfer extends Struct {
	external Pointer<Utf8> bank;
	external Pointer<Utf8> va_number;
	external Pointer<Utf8> bca;
	external Pointer<Utf8> permata;
}

class Midtrans {
	final DynamicLibrary dylib;

	Midtrans(String apiKey, String caInfo) {
		dylib = Platform.isAndroid
			? DynamicLibrary.open('libmidtrans.so')
			: DynamicLibrary.process();

		final apiKeyUtf8 = apiKey.toNativeUtf8();
		final caInfoUtf8 = caInfo.toNativeUtf8();
		final midtrans_init = dylib.lookupFunction
			<Void Function(Pointer<Utf8>, Pointer<Utf8>),
			void Function(Pointer<Utf8>, Pointer<Utf8>)>
				('midtrans_init');
		midtrans_init(apiKeyUtf8, caInfoUtf8);
		calloc.free(apiKeyUtf8);
		calloc.free(caInfoUtf8);
	}

	void charge<T extends Struct>(Pointer<T> payment,
			Pointer<MidtransTransaction> transaction,
			Array<Pointer<Utf8>> customFields) {
		if (payment is MidtransBanktransfer) {
			final midtrans_charge = dylib.lookupFunction
				<Void Function(Pointer<MidtransBanktransfer>,
						Pointer<MidtransTransaction>,
						Array<Pointer<Utf8>>),
			void Function(Pointer<MidtransBanktransfer>,
					Pointer<MidtransTransaction>,
					Array<Pointer<Utf8>>)>
				('midtrans_charge_banktransfer');
			midtrans_charge(payment, transaction, customFields);
			calloc.free((payment as MidtransBanktransfer).bank);
			if ((payment as MidtransBanktransfer).va_number
					!= null) {
				calloc.free((payment as MidtransBanktransfer)
						.va_number);
			}
			if ((payment as MidtransBanktransfer).bca != null) {
				calloc.free((payment as MidtransBanktransfer)
						.bca);
			}
			if ((payment as MidtransBanktransfer).permata != null) {
				calloc.free((payment as MidtransBanktransfer)
						.permata);
			}
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

import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

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

typedef MidtransStatus = Pointer<Utf8> Function(Pointer<Utf8>);

typedef MidtransChargeBanktransfer
= Pointer<Utf8> Function(MidtransBanktransfer, MidtransTransaction,
		Array<Pointer<Utf8>>);

class Midtrans {
	final dylib = Platform.isAndroid ? DynamicLibrary.open('libmidtrans.so')
		: DynamicLibrary.process();;

	Midtrans(String apiKey, String pem) {
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

	String status(String orderID) {
		final midtrans_status = dylib.lookupFunction<MidtransStatus,
		      MidtransStatus>('midtrans_status');
		final order_id = orderID.toNativeUtf8();
		final status = midtrans_status(order_id).toDartString();
		calloc.free(order_id);
		return status;
	}

	String chargeBanktransfer(MidtransBanktransfer payment,
			MidtransTransaction transaction,
			Array<Pointer<Utf8>> customFields) {
		final midtrans_charge = dylib.lookupFunction
			<MidtransChargeBanktransfer, MidtransChargeBanktransfer>
			('midtrans_charge_banktransfer');
		final va_number = midtrans_charge(payment, transaction,
				customFields).toDartString();
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
		return va_number;
	}

	void cleanup() {
		final midtrans_cleanup = dylib.lookupFunction<Void Function(),
		      void Function()>('midtrans_cleanup');
		midtrans_cleanup();
	}
}

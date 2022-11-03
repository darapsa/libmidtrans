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
typedef MidtransBanktransferNew = MidtransBanktransfer Function(Pointer<Utf8>);
typedef MidtransChargeBanktransfer
= Pointer<Utf8> Function(MidtransBanktransfer, MidtransTransaction);

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

	String chargeBanktransfer(String bank, String orderID,
			int grossAmount) {
		final midtrans_banktransfer_new = dylib.lookupFunction
			<MidtransBanktransferNew, MidtransBanktransferNew>
			('midtrans_banktransfer_new');
		final midtrans_transaction_new = dylib.lookupFunction
			<MidtransTransaction Function(Pointer<Utf8>, Long),
			MidtransTransaction Function(Pointer<Utf8>, int)>
			('midtrans_transaction_new');
		final midtrans_charge = dylib.lookupFunction
			<MidtransChargeBanktransfer, MidtransChargeBanktransfer>
			('midtrans_charge_banktransfer');
		final bankUtf8 = bank.toNativeUtf8();
		final order_id = orderID.toNativeUtf8();
		final vaNumber = midtrans_charge(
				midtrans_banktransfer_new(bankUtf8),
				midtrans_transaction_new(order_id, grossAmount))
			.toDartString();
		calloc.free(bankUtf8);
		calloc.free(order_id);
		return vaNumber;
	}

	void cleanup() {
		final midtrans_cleanup = dylib.lookupFunction<Void Function(),
		      void Function()>('midtrans_cleanup');
		midtrans_cleanup();
	}
}

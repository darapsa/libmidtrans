class MidtransTransaction extends Struct {
	external Pointer<Utf8> order_id;

	@Long()
	external long gross_amount;
}

typedef MidtransInitNative = Void Function(Pointer<Utf8> apiKey,
	Pointer<Utf8> caInfo)
typedef MidtransInit = void Function(Pointer<Utf8> apiKey, Pointer<Utf8> caInfo)

typedef MidtransChargeNative = Void Function(Int32 type, Pointer<Void>,
	Pointer<MidtransTransaction>, Array<Pointer<Utf8>>)
typedef MidtransCharge = void Function(int type, Pointer<Void>,
	Pointer<MidtransTransaction>, Array<Pointer<Utf8>>)

typedef MidtransCleanupNative = Void Function()
typedef MidtransCleanup = void Function()

void main() {
}

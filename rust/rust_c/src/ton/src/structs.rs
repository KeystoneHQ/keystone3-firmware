use common_rust_c::types::PtrString;


pub struct DisplayTonTransaction {
    amount: PtrString,
    action: PtrString,
    from: PtrString,
    to: PtrString,
    comment: PtrString,
    data_view: PtrString,
    raw_data: PtrString,
}


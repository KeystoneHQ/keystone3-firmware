use alloc::{string::String, vec::Vec};
use app_utils::impl_public_struct;

impl_public_struct!(ParsedPczt {
    transparent: Option<ParsedTransparent>,
    orchard: Option<ParsedOrchard>
});

impl_public_struct!(ParsedTransparent {
    from: Vec<ParsedFrom>,
    to: Vec<ParsedTo>
});

impl ParsedTransparent {
    pub fn add_from(&mut self, from: ParsedFrom) {
        self.from.push(from);
    }

    pub fn add_to(&mut self, to: ParsedTo) {
        self.to.push(to);
    }
}

impl_public_struct!(ParsedFrom {
    address: String,
    value: String,
    is_mine: bool
});

impl_public_struct!(ParsedTo {
    address: String,
    value: String,
    is_mine: bool,
    memo: Option<String>
});

impl_public_struct!(ParsedOrchard {
    from: Vec<ParsedFrom>,
    to: Vec<ParsedTo>
});

impl ParsedOrchard {
    pub fn add_from(&mut self, from: ParsedFrom) {
        self.from.push(from);
    }

    pub fn add_to(&mut self, to: ParsedTo) {
        self.to.push(to);
    }
}

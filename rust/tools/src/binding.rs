use third_party::cty::c_char;

extern "C" {
    pub fn PrintString(name: *mut c_char);
}

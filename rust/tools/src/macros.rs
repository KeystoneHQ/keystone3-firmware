#[macro_export]
macro_rules! debug {
    ($e: expr) => {{
        use rust_tools::binding::PrintString;
        use rust_tools::convert_c_char;
        unsafe {
            PrintString(convert_c_char($e));
        }
    }};
}

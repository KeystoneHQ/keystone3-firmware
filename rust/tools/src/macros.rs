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

#[macro_export]
macro_rules! debug_print {
    ($($arg:tt)*) => {{
        use rust_tools::binding::PrintString;
        use rust_tools::convert_c_char;
        use alloc::format;
        unsafe {
            PrintString(convert_c_char(format!($($arg)*)));
        }
    }};
}

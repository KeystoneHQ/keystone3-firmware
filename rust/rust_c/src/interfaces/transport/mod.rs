use transport::frame_parser;
use crate::interfaces::structs::SimpleResponse;
use crate::interfaces::utils::convert_c_char;
use crate::interfaces::types::PtrBytes;
use cty::c_char;

#[no_mangle]
pub extern "C" fn adpu_frame_parser(frame: PtrBytes, frame_len: u32) -> *mut SimpleResponse<c_char> {
    let inner_frame = unsafe { 
        core::slice::from_raw_parts(frame, frame_len as usize).to_vec() 
    };
    let output = frame_parser(inner_frame).join(" ");
    SimpleResponse::success(convert_c_char(output)).simple_c_ptr()
}
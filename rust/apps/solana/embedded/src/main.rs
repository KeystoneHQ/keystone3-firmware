#![feature(alloc_error_handler)]
#![no_std]
#![no_main]

extern crate alloc;
extern crate app_solana;

use alloc::string::ToString;
use alloc::vec;
use alloc::vec::Vec;
use core::alloc::Layout;

use alloc_cortex_m::CortexMHeap;
use app_solana::message::Message;
use app_solana::read::Read;
use hex::FromHex;

use cortex_m::asm;
use cortex_m_rt::entry;
use cortex_m_semihosting::{debug, hprintln};
use panic_halt as _;

#[global_allocator]
static ALLOCATOR: CortexMHeap = CortexMHeap::empty();

const HEAP_SIZE: usize = 1024 * 1024 * 10; // 10M

#[entry]
fn main() -> ! {
    hprintln!("heap size {}", HEAP_SIZE).unwrap();

    unsafe { ALLOCATOR.init(cortex_m_rt::heap_start() as usize, HEAP_SIZE) }
    {
        let message_valid = "01000103c8d842a2f17fd7aab608ce2ea535a6e958dffa20caf669b347b911c4171965530f957620b228bae2b94c82ddd4c093983a67365555b737ec7ddc1117e61c72e0000000000000000000000000000000000000000000000000000000000000000010295cc2f1f39f3604718496ea00676d6a72ec66ad09d926e3ece34f565f18d201020200010c0200000000e1f50500000000";
        let mut raw_valid = Vec::from_hex(message_valid).unwrap();
        let result_valid = Message::validate(&mut raw_valid);
        assert_eq!(result_valid, true);
    }
    hprintln!("heap size used {}, heap size free {}", ALLOCATOR.used(), ALLOCATOR.free()).unwrap();
    // exit QEMU
    // NOTE do not run this on hardware; it can corrupt OpenOCD state
    debug::exit(debug::EXIT_SUCCESS);

    loop {}
}

// define what happens in an Out Of Memory (OOM) condition
#[alloc_error_handler]
fn alloc_error(_layout: Layout) -> ! {
    hprintln!("alloc error").unwrap();
    debug::exit(debug::EXIT_FAILURE);
    asm::bkpt();

    loop {}
}


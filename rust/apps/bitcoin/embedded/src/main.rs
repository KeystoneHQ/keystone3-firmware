#![feature(alloc_error_handler)]
#![no_std]
#![no_main]

extern crate alloc;
extern crate app_bitcoin;

use alloc::string::ToString;
use alloc::vec;
use core::alloc::Layout;

use alloc_cortex_m::CortexMHeap;
use app_bitcoin::addresses::get_address;

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
        // Derive address
        let extended_pubkey = "xpub6BosfCnifzxcFwrSzQiqu2DBVTshkCXacvNsWGYJVVhhawA7d4R5WSWGFNbi8Aw6ZRc1brxMyWMzG3DSSSSoekkudhUd9yLb6qx39T9nMdj";
        hprintln!("heap size used {}, heap size free {}", ALLOCATOR.used(), ALLOCATOR.free()).unwrap();
        let address = get_address("m/0/0".to_string(), "BTC_LEGACY".to_string(), &extended_pubkey.to_string()).unwrap();
        hprintln!("heap size used {}, heap size free {}", ALLOCATOR.used(), ALLOCATOR.free()).unwrap();
        hprintln!("Address: {}", address).unwrap();
        assert_eq!(address.to_string(), "1LqBGSKuX5yYUonjxT5qGfpUsXKYYWeabA".to_string());
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


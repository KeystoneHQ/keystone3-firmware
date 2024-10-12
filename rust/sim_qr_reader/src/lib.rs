use anyhow::Result;
use image::DynamicImage;
use quircs::{Code, Quirc};
use screenshots::Screen;
use std::thread;
use std::time::{Duration, Instant};

use cocoa::appkit::NSScreen;
use cocoa::base::nil;
use cocoa::foundation::NSArray;

#[cfg(target_os = "macos")]
fn get_screen_scaling_factor() -> f64 {
    unsafe {
        let screens = NSScreen::screens(nil);
        let screen = NSArray::objectAtIndex(screens, 0);
        let scale_factor = NSScreen::backingScaleFactor(screen);
        scale_factor as f64
    }
}

#[cfg(not(target_os = "macos"))]
fn get_screen_scaling_factor() -> f64 {
    1.0 // Default value for non-macOS systems
}

pub fn continuously_read_qr_code_from_screen<G>(on_qr_code_detected: G, max_loop_count: u32) -> Result<()>
where
    G: Fn(&str) -> bool,
{
    let screens = Screen::all()?;
    let screen = screens.first().ok_or(anyhow::anyhow!("No screen found"))?;
    let mut decoder = Quirc::default();

    let scaling_factor = get_screen_scaling_factor();
    println!("Screen scaling factor: {}", scaling_factor);

    let mut qr_area: Option<(i32, i32, u32, u32)> = None;

    let mut capture_and_decode =
        |area: Option<(i32, i32, u32, u32)>| -> Result<(String, Option<(i32, i32, u32, u32)>)> {
            let image = match area {
                Some((x, y, width, height)) => {
                    let scaled_x = (x as f64 / scaling_factor) as i32;
                    let scaled_y = (y as f64 / scaling_factor) as i32;
                    let scaled_width = (width as f64 / scaling_factor) as u32;
                    let scaled_height = (height as f64 / scaling_factor) as u32;
                    println!(
                        "Capture area: ({}, {}, {}, {})",
                        scaled_x, scaled_y, scaled_width, scaled_height
                    );
                    screen.capture_area(scaled_x, scaled_y, scaled_width, scaled_height)?
                }
                None => screen.capture()?,
            };
            let dynamic_image = DynamicImage::ImageRgba8(image);
            let mut codes = decoder.identify(
                dynamic_image.width() as usize,
                dynamic_image.height() as usize,
                &dynamic_image.to_luma8(),
            );

            if let Some(Ok(code)) = codes.next() {
                let decoded = code.decode()?;
                let content = String::from_utf8(decoded.payload)?;
                let new_area = Some(get_qr_area(&code, &dynamic_image));
                Ok((content, new_area))
            } else {
                Err(anyhow::anyhow!("No QR code found on screen"))
            }
        };

    let interval = Duration::from_millis(100);

    let mut loop_count = 0;

    while loop_count < max_loop_count {
        println!("Loop count: {}, max: {}", loop_count, max_loop_count);
        match capture_and_decode(qr_area) {
            Ok((content, new_area)) => {
                if on_qr_code_detected(&content) {
                    break;
                }
                if let None = qr_area {
                    qr_area = new_area;
                }
                if let Some(area) = qr_area {
                    println!("QR code area determined: {:?}", area);
                }
            }
            Err(_) => {
                println!("No QR code detected, resetting scan area");
                qr_area = None; // Reset scan area
            }
        }
        thread::sleep(interval);
        loop_count += 1;
    }

    Ok(())
}

fn get_qr_area(code: &Code, image: &DynamicImage) -> (i32, i32, u32, u32) {
    let points = code.corners;
    let min_x = points.iter().map(|p| p.x).min().unwrap() as i32;
    let min_y = points.iter().map(|p| p.y).min().unwrap() as i32;
    let max_x = points.iter().map(|p| p.x).max().unwrap() as i32;
    let max_y = points.iter().map(|p| p.y).max().unwrap() as i32;

    let width = (max_x - min_x) as u32;
    let height = (max_y - min_y) as u32;

    // 添加一些边距
    let margin = 10;
    let x = (min_x - margin).max(0);
    let y = (min_y - margin).max(0);
    let width = (width + 2 * margin as u32).min(image.width() - x as u32);
    let height = (height + 2 * margin as u32).min(image.height() - y as u32);

    (x, y, width, height)
}

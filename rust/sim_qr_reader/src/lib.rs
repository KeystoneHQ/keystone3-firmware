#![allow(unexpected_cfgs)]

use anyhow::Result;
use image::DynamicImage;
#[cfg(target_os = "macos")]
use image::ImageOutputFormat;
use quircs::{Code, Quirc};
use screenshots::Screen;
#[cfg(target_os = "macos")]
use std::ffi::CStr;
#[cfg(target_os = "macos")]
use std::io::Cursor;
use std::thread;
use std::time::Duration;

#[cfg(target_os = "macos")]
use cocoa::{
    base::{id, nil},
    foundation::{NSAutoreleasePool, NSData, NSRect, NSUInteger},
};
#[cfg(target_os = "macos")]
use objc::{class, msg_send, sel, sel_impl};

type ScanArea = (i32, i32, u32, u32);

#[derive(Debug)]
enum ScanError {
    Capture(anyhow::Error),
    Decode(anyhow::Error),
    NoQrFound,
}

fn get_scale_factor(screen: &Screen) -> f64 {
    let scale_factor = screen.display_info.scale_factor as f64;
    if scale_factor.is_finite() && scale_factor > 0.0 {
        scale_factor
    } else {
        1.0
    }
}

#[cfg(target_os = "macos")]
fn ensure_screen_capture_permission() -> Result<()> {
    unsafe {
        if CGPreflightScreenCaptureAccess() {
            Ok(())
        } else {
            Err(anyhow::anyhow!(
                "macOS Screen Recording permission is missing. Run ./simulator.sh so the simulator opens in Terminal.app, then enable Terminal in System Settings > Privacy & Security > Screen Recording."
            ))
        }
    }
}

#[cfg(not(target_os = "macos"))]
fn ensure_screen_capture_permission() -> Result<()> {
    Ok(())
}

fn decode_qr_image(
    decoder: &mut Quirc,
    dynamic_image: &DynamicImage,
    scale_factor: f64,
) -> std::result::Result<(String, ScanArea), ScanError> {
    let codes = decoder.identify(
        dynamic_image.width() as usize,
        dynamic_image.height() as usize,
        &dynamic_image.to_luma8(),
    );

    let mut last_decode_error = None;

    for code in codes {
        let code = match code {
            Ok(code) => code,
            Err(err) => {
                last_decode_error = Some(anyhow::Error::from(err));
                continue;
            }
        };

        let decoded = match code.decode() {
            Ok(decoded) => decoded,
            Err(err) => {
                last_decode_error = Some(anyhow::Error::from(err));
                continue;
            }
        };

        let content =
            String::from_utf8(decoded.payload).map_err(|err| ScanError::Decode(err.into()))?;
        let new_area = get_qr_area(&code, dynamic_image, scale_factor);
        return Ok((content, new_area));
    }

    #[cfg(target_os = "macos")]
    match decode_qr_with_coreimage(dynamic_image, scale_factor) {
        Ok(Some(result)) => return Ok(result),
        Ok(None) => {}
        Err(err) => return Err(ScanError::Decode(err)),
    }

    if let Some(err) = last_decode_error {
        Err(ScanError::Decode(err))
    } else {
        Err(ScanError::NoQrFound)
    }
}

#[cfg(target_os = "macos")]
fn decode_qr_with_coreimage(
    dynamic_image: &DynamicImage,
    scale_factor: f64,
) -> Result<Option<(String, ScanArea)>> {
    unsafe {
        let _pool = NSAutoreleasePool::new(nil);

        let mut png_bytes = Vec::new();
        dynamic_image.write_to(&mut Cursor::new(&mut png_bytes), ImageOutputFormat::Png)?;

        let data: id = NSData::dataWithBytes_length_(
            nil,
            png_bytes.as_ptr() as *const _,
            png_bytes.len() as NSUInteger,
        );
        let ci_image: id = msg_send![class!(CIImage), imageWithData: data];
        if ci_image == nil {
            return Ok(None);
        }

        let detector_type: id = msg_send![
            class!(NSString),
            stringWithUTF8String: b"CIDetectorTypeQRCode\0".as_ptr()
                as *const std::os::raw::c_char
        ];
        let detector: id =
            msg_send![class!(CIDetector), detectorOfType: detector_type context: nil options: nil];
        if detector == nil {
            return Ok(None);
        }

        let features: id = msg_send![detector, featuresInImage: ci_image];
        let count: NSUInteger = msg_send![features, count];

        for i in 0..count {
            let feature: id = msg_send![features, objectAtIndex: i];
            let message: id = msg_send![feature, messageString];
            if message == nil {
                continue;
            }

            let content = nsstring_to_string(message)?;
            let bounds: NSRect = msg_send![feature, bounds];
            let new_area = nsrect_to_scan_area(bounds, dynamic_image, scale_factor);
            println!("QR decoded with CoreImage fallback");
            return Ok(Some((content, new_area)));
        }
    }

    Ok(None)
}

#[cfg(target_os = "macos")]
fn nsstring_to_string(string: id) -> Result<String> {
    unsafe {
        let bytes: *const std::os::raw::c_char = msg_send![string, UTF8String];
        if bytes.is_null() {
            return Err(anyhow::anyhow!("QR string conversion failed"));
        }

        Ok(CStr::from_ptr(bytes).to_string_lossy().into_owned())
    }
}

#[cfg(target_os = "macos")]
fn nsrect_to_scan_area(
    bounds: NSRect,
    dynamic_image: &DynamicImage,
    scale_factor: f64,
) -> ScanArea {
    let margin = 10.0;
    let min_x = (bounds.origin.x - margin).max(0.0);
    let min_y = (bounds.origin.y - margin).max(0.0);
    let max_x = (bounds.origin.x + bounds.size.width + margin).min(dynamic_image.width() as f64);
    let max_y = (bounds.origin.y + bounds.size.height + margin).min(dynamic_image.height() as f64);

    let x = (min_x / scale_factor).floor() as i32;
    let y = ((dynamic_image.height() as f64 - max_y) / scale_factor)
        .max(0.0)
        .floor() as i32;
    let max_x = (max_x / scale_factor).ceil() as i32;
    let max_y = ((dynamic_image.height() as f64 - min_y) / scale_factor)
        .max(0.0)
        .ceil() as i32;
    let width = (max_x - x).max(1) as u32;
    let height = (max_y - y).max(1) as u32;

    (x, y, width, height)
}

fn offset_scan_area(area: ScanArea, origin_x: i32, origin_y: i32) -> ScanArea {
    let (x, y, width, height) = area;
    (x + origin_x, y + origin_y, width, height)
}

fn capture_and_decode_screen(
    screen: &Screen,
    decoder: &mut Quirc,
    area: Option<ScanArea>,
) -> std::result::Result<(String, ScanArea), ScanError> {
    let (origin_x, origin_y) = area.map(|(x, y, _, _)| (x, y)).unwrap_or((0, 0));
    let image = match area {
        Some((x, y, width, height)) => {
            println!(
                "Capture area on screen[{}]: ({x}, {y}, {width}, {height})",
                screen.display_info.id
            );
            screen
                .capture_area(x, y, width, height)
                .map_err(ScanError::Capture)?
        }
        None => screen.capture().map_err(ScanError::Capture)?,
    };

    let dynamic_image = DynamicImage::ImageRgba8(image);
    let (content, new_area) = decode_qr_image(decoder, &dynamic_image, get_scale_factor(screen))?;
    Ok((content, offset_scan_area(new_area, origin_x, origin_y)))
}

pub fn continuously_read_qr_code_from_screen<G>(
    on_qr_code_detected: G,
    max_loop_count: u32,
) -> Result<()>
where
    G: Fn(&str) -> bool,
{
    ensure_screen_capture_permission()?;

    let screens = Screen::all()?;
    println!("Screens detected: {}", screens.len());
    for (i, screen) in screens.iter().enumerate() {
        println!(
            "  screen[{i}]: id={} x={} y={} w={} h={} scale={}",
            screen.display_info.id,
            screen.display_info.x,
            screen.display_info.y,
            screen.display_info.width,
            screen.display_info.height,
            screen.display_info.scale_factor
        );
    }
    if screens.is_empty() {
        return Err(anyhow::anyhow!("No screen found"));
    }

    let mut decoder = Quirc::default();
    let mut focused_screen_index: Option<usize> = None;
    let mut qr_area: Option<ScanArea> = None;
    let interval = Duration::from_millis(100);
    let mut loop_count = 0;

    while loop_count < max_loop_count {
        println!("Loop count: {loop_count}, max: {max_loop_count}");
        let attempt: Result<(usize, String, ScanArea)> = if let Some(screen_index) =
            focused_screen_index
        {
            let screen = &screens[screen_index];
            capture_and_decode_screen(screen, &mut decoder, qr_area)
                .map(|(content, new_area)| (screen_index, content, new_area))
                .map_err(|err| match err {
                    ScanError::Capture(err) => {
                        anyhow::anyhow!("Screen capture failed on screen[{screen_index}]: {err}")
                    }
                    ScanError::Decode(err) => {
                        anyhow::anyhow!("QR decode failed on screen[{screen_index}]: {err}")
                    }
                    ScanError::NoQrFound => {
                        anyhow::anyhow!("No QR code found on screen[{screen_index}]")
                    }
                })
        } else {
            let mut capture_errors = Vec::new();
            let mut decode_errors = Vec::new();
            let mut result = None;

            for (screen_index, screen) in screens.iter().enumerate() {
                match capture_and_decode_screen(screen, &mut decoder, None) {
                    Ok((content, new_area)) => {
                        result = Some((screen_index, content, new_area));
                        break;
                    }
                    Err(ScanError::NoQrFound) => {}
                    Err(ScanError::Capture(err)) => {
                        capture_errors.push(format!("screen[{screen_index}]: {err}"));
                    }
                    Err(ScanError::Decode(err)) => {
                        decode_errors.push(format!("screen[{screen_index}]: {err}"));
                    }
                }
            }

            if !capture_errors.is_empty() {
                println!("Screen capture errors: {}", capture_errors.join(" | "));
            }
            if !decode_errors.is_empty() {
                println!("QR decode errors: {}", decode_errors.join(" | "));
            }

            result.ok_or_else(|| anyhow::anyhow!("No QR code found on any display"))
        };

        match attempt {
            Ok((screen_index, content, new_area)) => {
                focused_screen_index = Some(screen_index);

                if on_qr_code_detected(&content) {
                    break;
                }

                qr_area = Some(new_area);
                println!("QR code area determined on screen[{screen_index}]: {new_area:?}");
            }
            Err(err) => {
                println!("Scan attempt failed: {err}");
                if focused_screen_index.is_some() || qr_area.is_some() {
                    println!("Resetting screen focus and scan area");
                }
                focused_screen_index = None;
                qr_area = None;
            }
        }
        thread::sleep(interval);
        loop_count += 1;
    }

    Ok(())
}

fn get_qr_area(code: &Code, image: &DynamicImage, scale_factor: f64) -> ScanArea {
    let points = code.corners;
    let min_x = points.iter().map(|p| p.x).min().unwrap();
    let min_y = points.iter().map(|p| p.y).min().unwrap();
    let max_x = points.iter().map(|p| p.x).max().unwrap();
    let max_y = points.iter().map(|p| p.y).max().unwrap();

    let margin = 10;
    let padded_min_x = (min_x - margin).max(0) as f64;
    let padded_min_y = (min_y - margin).max(0) as f64;
    let padded_max_x = (max_x + margin).min(image.width() as i32) as f64;
    let padded_max_y = (max_y + margin).min(image.height() as i32) as f64;

    let x = (padded_min_x / scale_factor).floor() as i32;
    let y = (padded_min_y / scale_factor).floor() as i32;
    let max_x = (padded_max_x / scale_factor).ceil() as i32;
    let max_y = (padded_max_y / scale_factor).ceil() as i32;
    let width = (max_x - x).max(1) as u32;
    let height = (max_y - y).max(1) as u32;

    (x, y, width, height)
}

#[cfg(target_os = "macos")]
#[link(name = "CoreGraphics", kind = "framework")]
unsafe extern "C" {
    fn CGPreflightScreenCaptureAccess() -> bool;
}

#[cfg(target_os = "macos")]
#[link(name = "CoreImage", kind = "framework")]
unsafe extern "C" {}

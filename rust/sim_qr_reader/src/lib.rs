use anyhow::Result;
use image::DynamicImage;
use screenshots::Screen;
use std::thread;
use std::time::Duration;

#[cfg(target_os = "macos")]
use cocoa::{appkit::NSScreen, base::nil, foundation::NSArray};

#[cfg(target_os = "windows")]
use windows::{
    Win32::Foundation::HWND,
    Win32::Graphics::Gdi::{GetDC, GetDeviceCaps, LOGPIXELSX},
    Win32::UI::HiDpi::{SetProcessDpiAwareness, PROCESS_SYSTEM_DPI_AWARE},
};

fn get_screen_scaling_factor() -> f64 {
    #[cfg(target_os = "macos")]
    unsafe {
        let screens = NSScreen::screens(nil);
        let screen = NSArray::objectAtIndex(screens, 0);

        NSScreen::backingScaleFactor(screen)
    }

    #[cfg(target_os = "windows")]
    unsafe {
        let hdc = GetDC(HWND::default());
        let dpi = GetDeviceCaps(hdc, LOGPIXELSX) as f64;
        // 96.0 default dpi
        dpi / 96.0 as f64
    }

    #[cfg(not(any(target_os = "macos", target_os = "windows")))]
    {
        1.0
    }
}

pub fn continuously_read_qr_code_from_screen<G>(
    on_qr_code_detected: G,
    max_loop_count: u32,
) -> Result<()>
where
    G: Fn(&str) -> bool,
{
    let screens = Screen::all()?;
    let screen = screens.first().ok_or(anyhow::anyhow!("No screen found"))?;

    let scaling_factor = get_screen_scaling_factor();
    println!("Screen scaling factor: {scaling_factor}");

    let mut qr_area: Option<(i32, i32, u32, u32)> = None;

    let capture_and_decode =
        |area: Option<(i32, i32, u32, u32)>| -> Result<(String, Option<(i32, i32, u32, u32)>)> {
            let image = match area {
                Some((x, y, width, height)) => {
                    let scaled_x = (x as f64 / scaling_factor) as i32;
                    let scaled_y = (y as f64 / scaling_factor) as i32;
                    let scaled_width = (width as f64 / scaling_factor) as u32;
                    let scaled_height = (height as f64 / scaling_factor) as u32;
                    println!(
                        "Capture area: ({scaled_x}, {scaled_y}, {scaled_width}, {scaled_height})"
                    );
                    screen.capture_area(scaled_x, scaled_y, scaled_width, scaled_height)?
                }
                None => screen.capture()?,
            };
            decode_image(&DynamicImage::ImageRgba8(image))
        };

    let interval = Duration::from_millis(100);

    let mut loop_count = 0;

    while loop_count < max_loop_count {
        println!("Loop count: {loop_count}, max: {max_loop_count}");
        match capture_and_decode(qr_area) {
            Ok((content, new_area)) => {
                if on_qr_code_detected(&content) {
                    break;
                }
                if qr_area.is_none() {
                    qr_area = new_area;
                }
                if let Some(area) = qr_area {
                    println!("QR code area determined: {area:?}");
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

type ScanArea = (i32, i32, u32, u32);

fn decode_image(image: &DynamicImage) -> Result<(String, Option<ScanArea>)> {
    let gray = image.to_luma8();
    let result = rxing::helpers::detect_in_luma_slice(
        gray.as_raw(),
        gray.width(),
        gray.height(),
        Some(rxing::BarcodeFormat::QR_CODE),
    )
    .map_err(|error| anyhow::anyhow!("No QR code decoded: {error}"))?;
    let area = get_qr_area(result.getPoints(), image.width(), image.height());
    Ok((result.getText().to_owned(), area))
}

fn get_qr_area(points: &[rxing::Point], width: u32, height: u32) -> Option<ScanArea> {
    if points.len() < 3 || points.iter().any(|p| !p.x.is_finite() || !p.y.is_finite()) {
        return None;
    }
    let min_x = points.iter().map(|p| p.x).fold(f32::INFINITY, f32::min);
    let min_y = points.iter().map(|p| p.y).fold(f32::INFINITY, f32::min);
    let max_x = points.iter().map(|p| p.x).fold(f32::NEG_INFINITY, f32::max);
    let max_y = points.iter().map(|p| p.y).fold(f32::NEG_INFINITY, f32::max);

    let margin = ((max_x - min_x).max(max_y - min_y) * 0.6).max(10.0);
    let x = (min_x - margin).floor().clamp(0.0, width as f32) as u32;
    let y = (min_y - margin).floor().clamp(0.0, height as f32) as u32;
    let right = (max_x + margin).ceil().clamp(0.0, width as f32) as u32;
    let bottom = (max_y + margin).ceil().clamp(0.0, height as f32) as u32;
    if right <= x || bottom <= y {
        return None;
    }
    Some((x as i32, y as i32, right - x, bottom - y))
}

#[cfg(test)]
mod tests {
    use super::*;
    use image::{GrayImage, Luma};
    use rxing::{qrcode::QRCodeWriter, BarcodeFormat, Writer};

    fn qr_image(content: &str) -> DynamicImage {
        let matrix = QRCodeWriter
            .encode(content, &BarcodeFormat::QR_CODE, 600, 600)
            .unwrap();
        DynamicImage::ImageLuma8(GrayImage::from_fn(600, 600, |x, y| {
            Luma([if matrix.get(x, y) { 0 } else { 255 }])
        }))
    }

    #[test]
    fn dense_qr_decodes_with_cached_crop() {
        let content = format!("ur:bytes/{}", "abcdef0123456789".repeat(50));
        let image = qr_image(&content);
        let (decoded, area) = decode_image(&image).unwrap();
        assert_eq!(decoded, content);
        let (x, y, width, height) = area.unwrap();
        let crop = image.crop_imm(x as u32, y as u32, width, height);
        assert_eq!(decode_image(&crop).unwrap().0, content);
    }

    #[test]
    fn small_qr_near_screen_edge_retains_decodable_capture_area() {
        let content = "screen QR regression";
        let qr = qr_image(content)
            .resize_exact(180, 180, image::imageops::FilterType::Nearest)
            .to_luma8();
        let mut screen = GrayImage::from_pixel(1200, 800, Luma([255]));
        image::imageops::replace(&mut screen, &qr, 1000, 10);
        let image = DynamicImage::ImageLuma8(screen);
        let (decoded, area) = decode_image(&image).unwrap();
        assert_eq!(decoded, content);
        let (x, y, width, height) = area.unwrap();
        assert!(width < image.width() && height < image.height());
        let crop = image.crop_imm(x as u32, y as u32, width, height);
        assert_eq!(decode_image(&crop).unwrap().0, content);
    }

    #[test]
    fn blank_image_is_not_a_qr_code() {
        let image = DynamicImage::ImageLuma8(GrayImage::from_pixel(320, 240, Luma([255])));
        assert!(decode_image(&image).is_err());
    }

    #[test]
    #[ignore = "requires a local screenshot via SIM_QR_TEST_IMAGE"]
    fn screenshot_regression() {
        let image = image::open(std::env::var("SIM_QR_TEST_IMAGE").unwrap()).unwrap();
        let (content, area) = decode_image(&image).unwrap();
        assert!(!content.is_empty());
        if let Some((x, y, width, height)) = area {
            let crop = image.crop_imm(x as u32, y as u32, width, height);
            assert_eq!(decode_image(&crop).unwrap().0, content);
        }
    }
}

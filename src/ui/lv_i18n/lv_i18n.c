#include "./lv_i18n.h"


////////////////////////////////////////////////////////////////////////////////
// Define plural operands
// http://unicode.org/reports/tr35/tr35-numbers.html#Operands

// Integer version, simplified

#define UNUSED(x) (void)(x)

static inline uint32_t op_n(int32_t val) { return (uint32_t)(val < 0 ? -val : val); }
static inline uint32_t op_i(uint32_t val) { return val; }
// always zero, when decimal part not exists.
static inline uint32_t op_v(uint32_t val) { UNUSED(val); return 0;}
static inline uint32_t op_w(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_f(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_t(uint32_t val) { UNUSED(val); return 0; }

static lv_i18n_phrase_t en_singulars[] = {
    {"Approve", "Approve"},
    {"Attention", "Attention"},
    {"Cancel", "Cancel"},
    {"Continue", "Continue"},
    {"Done", "Done"},
    {"Export", "Export"},
    {"FORGET", "FORGET"},
    {"Failed", "Failed"},
    {"Keystone", "Keystone"},
    {"OK", "OK"},
    {"Pending", "Pending"},
    {"Restart", "Restart"},
    {"Skip", "Skip"},
    {"Success", "Success"},
    {"Tutorial", "Tutorial"},
    {"Update", "Update"},
    {"Updating", "Updating"},
    {"about_info_device_uid", "Device UID"},
    {"about_info_export_log", "Export System Log"},
    {"about_info_export_to_sdcard", "Export Log to MicroSD Card"},
    {"about_info_fingerprint_firnware_version", "Fingerprint Firmware Version"},
    {"about_info_firmware_version", "Firmware Version"},
    {"about_info_result_export_failed", "Export Failed"},
    {"about_info_result_export_failed_desc_no_sdcard", "Please ensure you've inserted a MicroSD Card formatted in FAT32."},
    {"about_info_result_export_failed_desc_no_space", "Please make sure your MicroSD card has enough memory."},
    {"about_info_result_export_successful", "Export Successful"},
    {"about_info_result_export_successful_desc", "Your system log has been successfully exported to the MicroSD Card."},
    {"about_info_serial_number", "Serial Number"},
    {"about_keystone_discord", "Discord"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "Telegram"},
    {"about_keystone_telegram_url", "t.me/keystonewallet"},
    {"about_keystone_title", "About Keystone"},
    {"about_keystone_twitter", "Twitter"},
    {"about_keystone_twitter_url", "twitter.com/KeystoneWallet"},
    {"about_keystone_website", "WebSite"},
    {"about_keystone_website_url", "keyst.one"},
    {"about_terms_contact_us", "Contact Us"},
    {"about_terms_contact_us_desc", "If you have any questions or concerns, please email us at support@keyst.one."},
    {"about_terms_desc", "To access the full version of the TERMS OF USE, please visit the following link:"},
    {"about_terms_disclaimers", "DISCLAIMERS"},
    {"about_terms_disclaimers_desc", "The information provided is not financial advice. Seek professional advice before making any decisions."},
    {"about_terms_discontinuance_service", "Discontinuance of Service"},
    {"about_terms_discontinuance_service_desc", "We may modify or discontinue our services. Remember to back up your seed phrase to access your cryptocurrencies."},
    {"about_terms_eligibility", "Eligibility"},
    {"about_terms_eligibility_desc", "You must be 18 years old or above to access and use our Products or Services."},
    {"about_terms_indemnity", "Indemnity"},
    {"about_terms_indemnity_desc", "You must be 18 years old or above to access and use our Products or Services."},
    {"about_terms_law", "Governing Law and Dispute Resolution"},
    {"about_terms_law_desc", "The Terms are governed by Hong Kong SAR laws, and any dispute must be filed within one year."},
    {"about_terms_limitation", "Limitation of Liability & Disclaimer of Warranties"},
    {"about_terms_limitation_desc", "We provide our Services \"as is\" without warranties. We are not liable for any losses incurred while using our Products or Services."},
    {"about_terms_modification", "Modification of these Terms"},
    {"about_terms_modification_desc", "We reserve the right to change these Terms at our discretion."},
    {"about_terms_no_sensitive_information", "No Retrieval of Sensitive Information"},
    {"about_terms_no_sensitive_information_desc", "We do not store your sensitive information like passwords or seed phrases. Keep your credentials safe."},
    {"about_terms_ownership", "Ownership & Proprietary Rights"},
    {"about_terms_ownership_desc", "You are responsible for your actions while using the Products and Services."},
    {"about_terms_product_and_services", "Keystone Product & Services"},
    {"about_terms_product_and_services_desc", "Our hardware wallet securely manages cryptocurrencies."},
    {"about_terms_prohibited_conduct", "Prohibited Conduct"},
    {"about_terms_prohibited_product_desc", "Our Products and Services are protected by intellectual property laws."},
    {"about_terms_risks", "Risks"},
    {"about_terms_risks_desc", "Be aware of the risks associated with cryptocurrencies and technology vulnerabilities."},
    {"about_terms_subtitle", "Keystone Terms of Use"},
    {"about_terms_title", "Terms of Use"},
    {"about_terms_website_url", "https://keyst.one/terms"},
    {"change_passcode_mid_btn", "Enter Passcode"},
    {"change_passcode_reset_desc", "Writing Secure Element..."},
    {"change_passcode_reset_success_desc", "Your passcode has been reset successfully."},
    {"change_passcode_reset_success_title", "Reset Successful"},
    {"change_passcode_reset_title", "Resetting, Keep Devis ON"},
    {"change_passcode_warning_desc", "If forgotten, you'll have to verify the seed phrase for this wallet to reset the passcode."},
    {"change_passcode_warning_title", "Remember your Passcode"},
    {"connect_block_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_block_link", "https://keyst.one/t/3rd/block"},
    {"connect_block_qr_link", "https://keyst.one/t/3rd/block"},
    {"connect_block_qr_title", "BlockWallet (Extension)"},
    {"connect_block_t", "Tutorial"},
    {"connect_block_title", "BlockWallet (Extension)"},
    {"connect_bw_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_bw_link", "https://keyst.one/t/3rd/bw"},
    {"connect_bw_qr_link", "https://keyst.one/t/3rd/bw"},
    {"connect_bw_qr_title", "BlueWallet (Mobile)"},
    {"connect_bw_t", "Tutorial"},
    {"connect_bw_title", "BlueWallet (Mobile)"},
    {"connect_eternl_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_eternl_link", "https://keyst.one/t/3rd/eternl"},
    {"connect_eternl_qr_link", "https://keyst.one/t/3rd/eternl"},
    {"connect_eternl_qr_title", "Eternl (Web)"},
    {"connect_eternl_title", "Eternl (Web)"},
    {"connect_fewcha_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_fewcha_link", "https://keyst.one/t/3rd/fewcha"},
    {"connect_fewcha_qr_link", "https://keyst.one/t/3rd/fewcha"},
    {"connect_fewcha_qr_title", "Fewcha"},
    {"connect_fewcha_t", "Tutorial"},
    {"connect_fewcha_title", "Fewcha "},
    {"connect_keplr_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_keplr_link", "https://keyst.one/t/3rd/keplr"},
    {"connect_keplr_qr_link", "https://keyst.one/t/3rd/keplr"},
    {"connect_keplr_qr_title", "Keplr (Extension)"},
    {"connect_keplr_t", "Tutorial"},
    {"connect_keplr_title", "Keplr (Extension)"},
    {"connect_keyst_app_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_keyst_app_link", "https://keyst.one/t/3rd/keystone"},
    {"connect_keyst_app_qr_link", "https://keyst.one/t/3rd/keystone"},
    {"connect_keyst_app_qr_title", "Keystone Companion App"},
    {"connect_keyst_app_t", "Tutorial"},
    {"connect_keyst_app_title", "Keystone Companion App"},
    {"connect_mm_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_mm_link", "https://keyst.one/t/3rd/mm"},
    {"connect_mm_link2", "https://keyst.one/t/3rd/mmm"},
    {"connect_mm_qr_link", "https://keyst.one/t/3rd/mm"},
    {"connect_mm_qr_link2", "https://keyst.one/t/3rd/mmm"},
    {"connect_mm_qr_title", "MetaMask (Extension)"},
    {"connect_mm_qr_title2", "MetaMask (Mobile)"},
    {"connect_mm_t", "Tutorial"},
    {"connect_mm_title", "MetaMask (Extension)"},
    {"connect_mm_title2", "MetaMask (Mobile)"},
    {"connect_okx_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_okx_link", "https://keyst.one/t/3rd/okx"},
    {"connect_okx_link2", "https://keyst.one/t/3rd/okxm"},
    {"connect_okx_qr_link", "https://keyst.one/t/3rd/okx"},
    {"connect_okx_qr_link2", "https://keyst.one/t/3rd/okxm"},
    {"connect_okx_qr_title", "OKX Wallet (Extension)"},
    {"connect_okx_qr_title2", "OKX Wallet (Mobile)"},
    {"connect_okx_t", "Tutorial"},
    {"connect_okx_title", "OKX Wallet (Extension)"},
    {"connect_okx_title2", "OKX Wallet (Mobile)"},
    {"connect_petra_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_petra_link", "https://keyst.one/t/3rd/petra"},
    {"connect_petra_qr_link", "https://keyst.one/t/3rd/petra"},
    {"connect_petra_qr_title", "Petra (Extension)"},
    {"connect_petra_t", "Tutorial"},
    {"connect_petra_title", "Petra (Extension)"},
    {"connect_rabby_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_rabby_link", "https://keyst.one/t/3rd/rabby"},
    {"connect_rabby_qr_link", "https://keyst.one/t/3rd/rabby"},
    {"connect_rabby_qr_title", "Rabby (Extension)"},
    {"connect_rabby_t", "Tutorial"},
    {"connect_rabby_title", "Rabby (Extension)"},
    {"connect_safe_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_safe_link", "https://keyst.one/t/3rd/safe"},
    {"connect_safe_link2", "https://keyst.one/t/3rd/safem"},
    {"connect_safe_qr_link", "https://keyst.one/t/3rd/safe"},
    {"connect_safe_qr_link2", "https://keyst.one/t/3rd/safem"},
    {"connect_safe_qr_title", "Safe (Web)"},
    {"connect_safe_qr_title2", "Safe (Mobile)"},
    {"connect_safe_t", "Tutorial"},
    {"connect_safe_title", "Safe (Web)"},
    {"connect_safe_title2", "Safe (Mobile)"},
    {"connect_solflare_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_solflare_link", "https://keyst.one/t/3rd/solflare"},
    {"connect_solflare_qr_link", "https://keyst.one/t/3rd/solflare"},
    {"connect_solflare_qr_title", "Solflare"},
    {"connect_solflare_t", "Tutorial"},
    {"connect_solflare_title", "Solflare"},
    {"connect_sushi_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_sushi_link", "https://keyst.one/t/3rd/sushi"},
    {"connect_sushi_qr_link", "https://keyst.one/t/3rd/sushi"},
    {"connect_sushi_qr_title", "SushiSwap"},
    {"connect_sushi_t", "Tutorial"},
    {"connect_sushi_title", "SushiSwap"},
    {"connect_wallet_choose_wallet", "Choose Wallet"},
    {"connect_wallet_desc", "Scan the QR code with your software wallet"},
    {"connect_wallet_eternl_step1", "Select accounts you’d like to import on your Eternl wallet"},
    {"connect_wallet_eternl_step2", "Scan the QR code via your Keystone"},
    {"connect_wallet_eternl_step3", "Approve the request to generate a new QR code on your Keystone"},
    {"connect_wallet_eternl_step4", "Scan the QR code via Eternl wallet"},
    {"connect_wallet_instruction", "Follow the instructions below:"},
    {"connect_wallet_key_request_fmt", "%s Wallet wants to get your public key on your Keystone: "},
    {"connect_wallet_keystone_hint", "Select networks you’d like to manage in the software wallet"},
    {"connect_wallet_scan", "Scan the QR code with your software wallet"},
    {"connect_wallet_select_network", "Select Network"},
    {"connect_wallet_select_network_hint", "Select the networks you'd like to display within the software wallet."},
    {"connect_wallet_supported_networks", "Supported Networks"},
    {"connect_wallet_title", "Connect Wallet"},
    {"connect_wallet_upgrade_hint", "Please upgrade to the latest version for access to expanded software wallet compatibility."},
    {"connect_wallet_xpub_addresstype", "Address Type"},
    {"connect_wallet_xpub_qrformat", "QR Code Format"},
    {"connect_yearn_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_yearn_link", "https://keyst.one/t/3rd/yearn"},
    {"connect_yearn_qr_link", "https://keyst.one/t/3rd/yearn"},
    {"connect_yearn_qr_title", "Yearn"},
    {"connect_yearn_t", "Tutorial"},
    {"connect_yearn_title", "Yearn"},
    {"connect_zapper_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_zapper_link", "https://keyst.one/t/3rd/zapper"},
    {"connect_zapper_qr_link", "https://keyst.one/t/3rd/zapper"},
    {"connect_zapper_qr_title", "Zapper"},
    {"connect_zapper_t", "Tutorial"},
    {"connect_zapper_title", "Zapper"},
    {"create_wallet_generating_desc", "Writing Secure Element..."},
    {"create_wallet_generating_title", "Creating Wallet, Keep Device ON"},
    {"derivation_path_address", "Address"},
    {"derivation_path_address_eg", "Addresses eg."},
    {"derivation_path_btc_1_desc", "Native SegWit: Modern format, lower fees, starts with \"bc1\"."},
    {"derivation_path_btc_2_desc", "Nested SegWit: Compatible, medium fees, starts with \"3\"."},
    {"derivation_path_btc_3_desc", "Legacy: Original format, higher fees, starts with \"1\"."},
    {"derivation_path_change", "Change Derivation Path"},
    {"derivation_path_eth_ledger_legacy_desc", "Choose this path if you are managing your digital assets with Ledger Legacy."},
    {"derivation_path_eth_ledger_live_desc", "Choose this path if you intend to import a seed phrase from Ledger Live."},
    {"derivation_path_eth_standard_desc", "Recommended. Widely adopted across numerous software wallets"},
    {"derivation_path_select_btc", "Select the address type you’d like to use for Bitcoin"},
    {"derivation_path_select_eth", "Select the derivation path you’d like to use for Ethereum"},
    {"derivation_path_select_sol", "Select the derivation path you’d like to use for Solana"},
    {"derivation_path_sol_1_desc", "Widely adopted across numerous Solana wallets. An example of such a wallet is Solflare."},
    {"derivation_path_sol_2_desc", "Transition to this path if you manage your digital assets with software wallets like Sollet / MathWallet."},
    {"derivation_path_sol_3_desc", "Transition to this path if you manage your digital assets with software wallets like Phantom / Exodus."},
    {"device_info_title", "Device Info"},
    {"device_setting_about_title", "About"},
    {"device_setting_about_title_desc", "v1.0.2"},
    {"device_setting_connection_desc", "USB / MicroSD Card"},
    {"device_setting_connection_title", "Connection"},
    {"device_setting_mid_btn", "Device Settings"},
    {"device_setting_system_setting_desc", "Language / Screen / Reset..."},
    {"device_setting_system_setting_title", "System Settings"},
    {"device_setting_wallet_setting_desc", "Name / Passcode / Passphrase..."},
    {"device_setting_wallet_setting_title", "Wallet Settings"},
    {"device_settings_connection_desc1", "When disabled, the usb can only be used for charging battery"},
    {"device_settings_connection_sub_title1", "Data transfer with USB"},
    {"device_settings_connection_title1", "Connection"},
    {"enter_passcode", "Please Enter Passcode"},
    {"error_box_duplicated_seed_phrase", "Duplicate Seed Phrase"},
    {"error_box_duplicated_seed_phrase_desc", "This phrase you typed is already used in a wallet account, please use another mnemonic to import."},
    {"error_box_firmware_not_detected", "Firmware Not Detected"},
    {"error_box_firmware_not_detected_desc", "Please ensure that your MicroSD card is formatted in FAT32 and contains the firmware 'keystone3.bin'."},
    {"error_box_invalid_seed_phrase", "Invalid Seed Phrase"},
    {"error_box_invalid_seed_phrase_desc", "The seed phrase you've entered is invalid. Please re-verify your backup and try again."},
    {"error_box_low_power", "Low Battery"},
    {"error_box_low_power_desc", "The device needs a minimum of 20% battery life to continue the process."},
    {"error_box_mnemonic_not_match_wallet", "Seed Phrase Mismatch"},
    {"error_box_mnemonic_not_match_wallet_desc", "The seed phrase is incorrect. Please re-verify and try again."},
    {"error_unknown_error", "Unknown Error"},
    {"error_unknown_error_desc", "There is an unknown problem with the device and it is unavailable. Please wipe the device to try to restart the device. If the problem still exists, please contact our customer service team."},
    {"fingerprint_passcode_fingerprint_setting", "Fingerprint Settings"},
    {"fingerprint_passcode_mid_btn", "Fingerprint & Passcode"},
    {"fingerprint_passcode_reset_passcode", "Reset Passcode"},
    {"firmware_update_deny_desc", "You need to unlock your device to upgrade the firmware version."},
    {"firmware_update_deny_input_password", "Enter Password"},
    {"firmware_update_deny_input_password_title", "Please Enter Password"},
    {"firmware_update_deny_title", "Unlock Device Required"},
    {"firmware_update_desc", "To take advantage of the latest features, make sure to update your firmware to the most recent version."},
    {"firmware_update_sd_copying_desc", "The copying process of the latest firmware from the MicroSD card may take 15 to 45 seconds."},
    {"firmware_update_sd_copying_title", "Copying"},
    {"firmware_update_sd_desc1", "Ensure that your Keystone has at least 20% battery life remaining."},
    {"firmware_update_sd_desc2", "Navigate to Keystone's firmware update page using your computer or mobile."},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Transfer the firmware file (keystone3.bin) to your Keystone using a MicroSD card formatted in FAT32."},
    {"firmware_update_sd_desc4", "Tap the #F5870A Update# button below to initiate the process."},
    {"firmware_update_sd_dialog_desc", "A new firmware version is available. Do you want to update your device's firmware to the new version?"},
    {"firmware_update_sd_dialog_title", "Update Available"},
    {"firmware_update_sd_failed_access_desc", "Please ensure that you have properly inserted the MicroSD Card."},
    {"firmware_update_sd_failed_access_title", "MicroSD Card Not Detected"},
    {"firmware_update_sd_not_detected_desc", "Please ensure that your MicroSD card is formatted in FAT32 and contains the firmware 'keystone3.bin'."},
    {"firmware_update_sd_not_detected_title", "Firmware Not Detected"},
    {"firmware_update_sd_title", "Update via MicroSD"},
    {"firmware_update_sd_updating_desc", "Takes around 5 seconds"},
    {"firmware_update_sd_updating_title", "Updating"},
    {"firmware_update_title", "Firmware Update"},
    {"firmware_update_updating_desc", "Takes around 5 seconds"},
    {"firmware_update_updating_title", "Updating"},
    {"firmware_update_usb_button_1", "Not Now"},
    {"firmware_update_usb_button_2", "Connect"},
    {"firmware_update_usb_connect_info_desc", "Once connected, the external device will gain permission to transfer data to your Keystone."},
    {"firmware_update_usb_connect_info_title", "Connect to This Device?"},
    {"firmware_update_usb_desc1", "Ensure that your Keystone has at least 20% battery life remaining."},
    {"firmware_update_usb_desc2", "Navigate to Keystone's firmware update page using your computer or mobile."},
    {"firmware_update_usb_desc2_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_desc3", "Connect your Keystone to your computer using a USB-C cable."},
    {"firmware_update_usb_desc4", "Click the #F5870A Install Update# button on the webpage and follow the instructions to install the latest firmware."},
    {"firmware_update_usb_desc5", "Do not disconnect the USB cable during the installation process."},
    {"firmware_update_usb_low_button", "OK"},
    {"firmware_update_usb_low_desc", "The device needs a minimum of 20% battery life to continue the process."},
    {"firmware_update_usb_low_title", "Low Battery"},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "Firmware Update"},
    {"firmware_update_usb_title", "USB Update"},
    {"firmware_update_usb_title2", "#F5870A Caution#"},
    {"firmware_update_usb_updating_hint", "Do not disconnect the USB cable during the installation process."},
    {"firmware_update_via_sd", "Via MicroSD Card"},
    {"firmware_update_via_usb", "Via USB"},
    {"forget_password_cancel", "Cancel forget password?"},
    {"forgot_password_reset_passcode_intro_button", "Reset Passcode"},
    {"forgot_password_reset_passcode_intro_desc", "Verify the seed phrase associated with this wallet to reset the passcode."},
    {"forgot_password_reset_passcode_intro_title", "Forgot passcode?"},
    {"generating_qr_codes", "Generating QR Codes"},
    {"got_it", "Got It"},
    {"home_manage_assets", "Manage Assets"},
    {"home_more_connect_wallet", "Connect Software Wallet"},
    {"home_more_device_setting", "Device Settings"},
    {"home_select_coin_count_fmt", "#F5870A %d#/%d"},
    {"home_upgrade_hint", "Please upgrade to the latest version for access to expanded crypto compatibility."},
    {"import_wallet_choose_method", "Choose Import Method"},
    {"import_wallet_duplicated_share_desc", "You’ve already checked this share, please use another share to continue."},
    {"import_wallet_duplicated_share_title", "Duplicate Share"},
    {"import_wallet_invalid_phrase_desc", "The phrase you typed is invalid. Please check your backup and try again"},
    {"import_wallet_invalid_phrase_title", "Invalid Seed Phrase"},
    {"import_wallet_invalid_phrase_title_desc", "The seed phrase you've entered is invalid. Please re-verify your backup and try again."},
    {"import_wallet_phrase_12words", "12 Words"},
    {"import_wallet_phrase_18words", "18 Words"},
    {"import_wallet_phrase_24words", "24 Words"},
    {"import_wallet_phrase_clear_btn", "Clear"},
    {"import_wallet_phrase_desc", "Enter your seed phrase in the blanks provided below."},
    {"import_wallet_phrase_title", "Import Your Seed"},
    {"import_wallet_phrase_words_title", "Seed Phrase Count"},
    {"import_wallet_shamir_backup", "Shamir Backup"},
    {"import_wallet_shamir_backup_desc", "You'll need a couple of seed phrase\nshares to recover your wallet"},
    {"import_wallet_share_success_desc", "This share of your seed phrase matches your wallet."},
    {"import_wallet_single_backup_desc", "Recover your wallet with the specific\nseed phrase"},
    {"import_wallet_single_phrase", "Single Secret Phrase"},
    {"import_wallet_single_phrase_desc", "You'll need a 12/18/24 seed phrase\nto recover your wallet."},
    {"import_wallet_ssb_20words", "20 Words"},
    {"import_wallet_ssb_33words", "33 Words"},
    {"import_wallet_ssb_cancel_btn_1", "Not Now"},
    {"import_wallet_ssb_cancel_btn_2", "Quit"},
    {"import_wallet_ssb_cancel_desc", "Upon cancellation, you will be required to re-enter all the Shares again."},
    {"import_wallet_ssb_cancel_title", "Cancel Import Wallet?"},
    {"import_wallet_ssb_desc_fmt", "Enter your  #F5870A %d#-word seed phrase for Share #F5870A %d# in the blanks provided below."},
    {"import_wallet_ssb_incorrect_title", "Duplicate Share"},
    {"import_wallet_ssb_notbelong_desc", "The Share you've entered doesn't correspond to this backup. Please re-verify your backup and try again."},
    {"import_wallet_ssb_notbelong_title", "Share Mismatch"},
    {"import_wallet_ssb_repeat_desc", "The Share entered has already been previously inputted. Please submit a different Share."},
    {"import_wallet_ssb_step_fmt", "%d of %d"},
    {"import_wallet_ssb_title_fmt", "Share #F5870A %d#"},
    {"import_wallet_ssb_words_title", "Seed Phrase Count"},
    {"language_desc", "Select your language"},
    {"language_option1", "English"},
    {"language_option2", "简体中文"},
    {"language_option3", "Русский язык"},
    {"language_option4", "Español"},
    {"language_option5", "한국인"},
    {"language_title", "Language"},
    {"low_battery_pop_up_desc", "The device needs a minimum of 20% battery life to continue the process."},
    {"low_battery_pop_up_title", "Low Battery"},
    {"not_now", "Not Now"},
    {"passphrase_access_switch_desc", "Create a passphrase shortcut for device boot-up"},
    {"passphrase_access_switch_title", "Passphrase Quick Access"},
    {"passphrase_enter_input", "Input passphrase"},
    {"passphrase_enter_passcode", "Passphrase"},
    {"passphrase_enter_repeat", "Confirm passphrase"},
    {"passphrase_error_not_match", "Passphrase mismatch"},
    {"passphrase_error_too_long", "input length cannot exceed 128 characters"},
    {"passphrase_learn_more_desc1", "A passphrase offers an added layer of security in addition to your existing seed phrase."},
    {"passphrase_learn_more_desc2", "Each unique passphrase generates a different wallet."},
    {"passphrase_learn_more_desc3", "To recover your wallet, both the passphrase and seed phrase are required."},
    {"passphrase_learn_more_desc4", "Forgetting the passphrase can result in the loss of access to your digital assets."},
    {"passphrase_learn_more_link", "Learn More"},
    {"passphrase_learn_more_qr_link", "https://keyst.one/t/3rd/passphrase"},
    {"passphrase_learn_more_qr_title", "What is a Passphrase?"},
    {"passphrase_learn_more_title", "What is a Passphrase?"},
    {"password_error_cannot_verify_fingerprint", "Couldn’t verify fingerprint"},
    {"password_error_duplicated_pincode", "Duplicate PIN code detected. Please use a different one."},
    {"password_error_fingerprint_attempts_exceed", "Too many attempts. Please enter your passcode to unlock the device."},
    {"password_error_not_match", "Passcode mismatch"},
    {"password_error_too_long", "The input cannot exceed 128 characters"},
    {"password_error_too_short", "Password must be at least 6 characters"},
    {"password_error_too_weak", "Set a strong password"},
    {"password_input_desc", "Enter your password"},
    {"password_label", "PASSWORD"},
    {"pin_code", "PIN CODE"},
    {"power_off", "Power Off"},
    {"prepare_wallet_first_step", "New chains support detected"},
    {"prepare_wallet_hint", "Preparing Wallet"},
    {"prepare_wallet_second_step", "Setting up support for new coins..."},
    {"prepare_wallet_third_step", "Generating extended public key..."},
    {"purpose_desc", "Create a new wallet or import an existing wallet using it's seed phrase."},
    {"purpose_import_wallet", "Import Wallet"},
    {"purpose_new_wallet", "Create Wallet"},
    {"purpose_title", "New Wallet"},
    {"receive_ada_address_detail_path", "Path"},
    {"receive_ada_base_address", "Address"},
    {"receive_ada_enterprise_address", "Address (Not Delegated)"},
    {"receive_ada_more_t", "Tutorial"},
    {"receive_ada_more_t_desc1", "1. Payment & Stake Keys: In Cardano, every account has a Payment Key for regular ADA transactions (sending and receiving) and a Stake Key for staking and receiving rewards."},
    {"receive_ada_more_t_desc2", "2. Base Address: A Base Address is derived from both the Payment Key and Stake Key. It can be used for both regular transactions and staking. Also known as \"External Addresses (Delegated).\""},
    {"receive_ada_more_t_desc3", "3. Enterprise Address: This address only contains the Payment Key and is used solely for regular transactions, not for staking. It's designed for \"business\" scenarios that don't involve staking, like exchanges. Also known as \"External Addresses (Not Delegated).\""},
    {"receive_ada_more_t_desc4", "4. Stake & Reward Addresses: The Stake Key is used for staking, and the associated Stake Address is also called a Reward Address, used to receive staking rewards."},
    {"receive_ada_more_t_link1", "Learn More"},
    {"receive_ada_more_t_qr_link1", "https://keyst.one/t/3rd/faq"},
    {"receive_ada_more_t_qr_title1", "Key Concepts in Cardano's ADA Addresses"},
    {"receive_ada_more_t_title1", "Multiple Cardano Accounts on Keystone"},
    {"receive_ada_more_t_title1_1", "On the Cardano blockchain, we provide 24 accounts, and each account can generate numerous addresses for your use. You can easily switch between the accounts you need.\n"},
    {"receive_ada_more_t_title2", "Key Concepts in Cardano's ADA Addresses"},
    {"receive_ada_show_address_detail", "Show Address Detail"},
    {"receive_ada_stake_address", "Reward Address"},
    {"receive_btc_alert_button", "Got It"},
    {"receive_btc_alert_desc", "This address is exclusively for BTC transactions only. Sending other types of digital assets to this address will result in their loss."},
    {"receive_btc_alert_title", "Attention"},
    {"receive_btc_more_address_settings", "Address Settings"},
    {"receive_btc_more_export_xpub", "Export XPub"},
    {"receive_btc_more_t", "Tutorial"},
    {"receive_btc_more_t_desc1", "Bitcoin (BTC) uses three address formats for receiving funds:\nNative SegWit is the most efficient and secure Bitcoin address format. It provides cost savings and improved security compared to other traditional address formats, typically starting with \"bc1\"\nLegacy format is one of the earliest versions of Bitcoin, typically starting with \"1\"\nNested SegWit is a solution designed to facilitate the transition to Native SegWit in a smooth manner, typically starting with \"3\""},
    {"receive_btc_more_t_desc2", "Yes, the three distinct Bitcoin address formats can be used for transferring funds among each other. However, it’s important to keep in mind the following aspects:\n1. Differing transaction fees: The choice of address format can influence transaction fees, with Native SegWit addresses generally having lower fees.\n2. Wallet and exchange compatibility: Make sure that the wallet or exchange you are using supports your chosen address format. Some wallets may only be compatible with specific address formats."},
    {"receive_btc_more_t_desc3", "1. Privacy: Reusing addresses increases transaction traceability, endangering privacy. New addresses help maintain transaction privacy.\n2. Transaction Efficiency: Multiple UTXOs linked to one address can raise costs for consolidation, impacting wallet efficiency.\n3. Security: Repeated address use heightens the risk of private key exposure, potentially leading to losses if compromised.\nIn short, not reusing addresses safeguards privacy, optimizes transactions, and reduces security risks within the UTXO model."},
    {"receive_btc_more_t_link1", "Learn More"},
    {"receive_btc_more_t_link2", "Learn More"},
    {"receive_btc_more_t_link3", "Learn More"},
    {"receive_btc_more_t_qr_link1", "https://keyst.one/t/3rd/faq"},
    {"receive_btc_more_t_qr_link2", "https://keyst.one/t/3rd/faq"},
    {"receive_btc_more_t_qr_link3", "https://keyst.one/t/3rd/faq"},
    {"receive_btc_more_t_qr_title1", "What are the three different address formats for BTC?"},
    {"receive_btc_more_t_qr_title2", "Can the three different address formats be used to transfer funds to each other?"},
    {"receive_btc_more_t_qr_title3", "Benefits of not reusing addresses"},
    {"receive_btc_more_t_title1", "What are the three different address formats for BTC?"},
    {"receive_btc_more_t_title2", "Can the three different address formats be used to transfer funds to each other?"},
    {"receive_btc_more_t_title3", "Benefits of not reusing addresses"},
    {"receive_btc_receive_change_address_limit", "Can't exceed 999,999,999"},
    {"receive_btc_receive_change_address_title", "Go to"},
    {"receive_btc_receive_main_button", "Generate New Address"},
    {"receive_btc_receive_main_title", "Receive BTC"},
    {"receive_btc_receive_switch_title", "Switch Address"},
    {"receive_coin_fmt", "Receive %s"},
    {"receive_coin_hint_fmt", "This address is only for %s, other digital assets sent to this address will be lost."},
    {"receive_eth_alert_button", "Got It"},
    {"receive_eth_alert_desc", "This address is only for ETH and EVM ERC-20 tokens, other digital assets sent to this address will be lost."},
    {"receive_eth_alert_title", "Attention"},
    {"receive_eth_more_derivation_path", "Change Derivation Path"},
    {"receive_eth_more_derivation_path_bip", "BIP 44 Standard"},
    {"receive_eth_more_derivation_path_desc", "Recommend. Most commonly used in many software wallets."},
    {"receive_eth_more_derivation_path_desc2", "Select this path for Ledger Live asset management."},
    {"receive_eth_more_derivation_path_desc3", "Select this path for Ledger Legacy asset management."},
    {"receive_eth_more_derivation_path_ledger_legacy", "Ledger Legacy"},
    {"receive_eth_more_derivation_path_ledger_live", "Ledger Live"},
    {"receive_eth_more_derivation_path_title", "Change Derivation Path"},
    {"receive_eth_more_derivation_path_title2", "Accounts eg:"},
    {"receive_eth_more_t", "Tutorial"},
    {"receive_eth_more_t_desc1", "1. Standard Path: This path is widely employed by numerous software wallets for address generation. Examples of such wallets encompass MetaMask, Rabby, BitKeep, and Core Wallet.\n2. Ledger Live: Choose this path if you intend to import a seed phrase from Ledger Live. Please note that Keystone supports synchronizing only the initial 10 addresses using this format.\n3. Ledger Legacy: Transition to this path if you manage your digital assets with Ledger Legacy"},
    {"receive_eth_more_t_link1", "Learn More"},
    {"receive_eth_more_t_qr_link1", "https://keyst.one/t/3rd/faq"},
    {"receive_eth_more_t_qr_title1", "Exploring Differences: Standard, Ledger Live, and Legacy Derivation Paths in Ethereum"},
    {"receive_eth_more_t_title0", "Exploring Differences: Standard, Ledger Live, and Legacy Derivation Paths in Ethereum"},
    {"receive_eth_more_t_title1", "1. Standard Path: This path is widely employed by numerous software wallets for address generation. Examples of such wallets encompass MetaMask, Rabby, BitKeep, and Core Wallet.\n2. Ledger Live: Choose this path if you intend to import a seed phrase from Ledger Live. Please note that Keystone supports synchronizing only the initial 10 addresses using this format.\n3. Ledger Legacy: Transition to this path if you manage your digital assets with Ledger Legacy"},
    {"receive_eth_receive_main_button", "Generate New Address"},
    {"receive_eth_receive_main_title", "Receive ETH"},
    {"receive_generate_new_address", "Generate New Address"},
    {"receive_sol_more_t", "Tutorial"},
    {"receive_sol_more_t_desc1", "1. Solflare: Widely adopted across numerous Solana wallets. An example of such a wallet is Solflare.\n2. Sollet / MathWallet: Choose this path if you intend to import a seed phrase from Sollet / MathWallet. Please note that Keystone supports synchronizing only the initial 10 addresses using this format.\n3. Phantom / Exodus: Transition to this path if you manage your digital assets with software wallets like Phantom / Exodus."},
    {"receive_sol_more_t_link1", "Learn More"},
    {"receive_sol_more_t_qr_link1", "https://keyst.one/t/3rd/faq"},
    {"receive_sol_more_t_qr_title1", "Exploring Solana Path Options"},
    {"receive_sol_more_t_title1", "Exploring Solana Path Options"},
    {"receive_trx_hint", "This address is only for TRX, TRC-20 tokens and TRC-10 tokens, other digital assets sent to this address will be lost."},
    {"repeat_passcode_desc", "Double confirm the PIN code you've entered."},
    {"repeat_passcode_title", "Re-Enter PIN Code"},
    {"scan_qr_code_error_invalid_qrcode", "Invalid QR Code"},
    {"scan_qr_code_error_invalid_qrcode_desc", "QR code data not recognized. Please try again."},
    {"scan_qr_code_invalid_b_desc", "The transaction does not belong to the current wallet. Please ensure you are using the correct wallet to authorize the transaction."},
    {"scan_qr_code_invalid_b_title", "Unpermitted Authorization"},
    {"scan_qr_code_invalid_c_desc", "Keystone cannot facilitate transaction signing for the current address path. Please try again using a different address path."},
    {"scan_qr_code_invalid_c_title", "Unsupported Path"},
    {"scan_qr_code_sign_fingerprint_enter_passcode", "Enter Passcode"},
    {"scan_qr_code_sign_fingerprint_verify_fingerprint", "Verify Fingerprint"},
    {"scan_qr_code_sign_unsigned_content_fingerprint_failed_desc", "Verification failed. Please try again!"},
    {"scan_qr_code_sign_unsigned_content_fingerprint_failed_desc2", "If the verification fails again, the fingerprint signing for transactions will automatically be disabled."},
    {"scan_qr_code_sign_unsigned_content_frame", "Swipe to confirm"},
    {"scan_qr_code_signing_desc", "Signing"},
    {"seed_check_mid_btn", "Seed Phrase Check"},
    {"seed_check_share_phrase", "Shamir Backup"},
    {"seed_check_share_phrase_title", "Enter your seed phrase to verify if it matches your current wallet."},
    {"seed_check_single_phrase", "Standard Seed Phrase"},
    {"seed_check_single_phrase_title", "Enter Your Seed"},
    {"seed_check_verify_match_desc", "Your seed has been validated and successfully verified."},
    {"seed_check_verify_match_title", "Verification Successful"},
    {"seed_check_verify_not_match_desc", "The seed phrase is incorrect. Please re-verify and try again."},
    {"seed_check_verify_not_match_title", "Verification Failed"},
    {"seed_check_wait_verify", "Verifying"},
    {"self_destruction_desc", "Physical attack detected, all sensitive information on this device has been completely erased and this device can no longer be usable."},
    {"self_destruction_hint", "Contact us if any questions:"},
    {"self_destruction_title", "Device No Longer Usable"},
    {"set_passcode_desc", "This PIN code will be used to unlock your wallet and authorize transactions."},
    {"set_passcode_title", "Set a PIN Code"},
    {"shamir_backup", "Shamir Backup"},
    {"shamir_phrase_backup_desc", "Write down your Share 1 phrase and keep it properly."},
    {"shamir_phrase_backup_title", "Backup Your Phrase"},
    {"shamir_phrase_cancel_create_btn", "Quit"},
    {"shamir_phrase_cancel_create_desc", "Are you sure you want to cancel the setup flow? Any confirmed Shares will be lost upon cancellation."},
    {"shamir_phrase_cancel_create_title", "Quit Wallet Setup?"},
    {"shamir_phrase_confirm_desc", "Select the words below in the correct order of your Share #F5870A 1# phrase to validate your seed phrase."},
    {"shamir_phrase_confirm_title", "Confirm Your Phrase"},
    {"shamir_phrase_continue_btn", "Continue"},
    {"shamir_phrase_custodian_desc", "Please confirm you are the custodian of the Share"},
    {"shamir_phrase_custodian_title", "Share Share #F5870A 1#/N"},
    {"shamir_phrase_desc", "Shamir Backup"},
    {"shamir_phrase_not_match_desc", "The seed phrase is incorrect. Please re-verify and try again."},
    {"shamir_phrase_not_match_title", "Seed Phrase Mismatch"},
    {"shamir_phrase_notice_desc1", "Never share your seed phrase with anyone else, as it grants access to your assets."},
    {"shamir_phrase_notice_desc2", "Ensure there are no onlookers or cameras when recording your seed phrase."},
    {"shamir_phrase_notice_title", "Check Your Surroundings"},
    {"shamir_phrase_number", "Number of Shares"},
    {"shamir_phrase_share_backup_notice_fmt", "Write down your Share #F5870A %d# phrase and keep it properly."},
    {"shamir_phrase_share_confirm_notice_fmt", "Select the words below in the correct order of your Share #F5870A %d#/%d phrase to validate your seed phrase."},
    {"shamir_phrase_share_notice_fmt", "Please confirm you are the custodian of the\nShare #%d"},
    {"shamir_phrase_share_number_fmt", "Share #F5870A %d#/%d"},
    {"shamir_phrase_threold", "Threshold"},
    {"shamir_phrase_verify_success_desc1", "The seed phrase for this Share has been validated, please proceed to the next Share."},
    {"shamir_phrase_verify_success_desc2", "Tap the button below and hand the Keystone over to the custodian of Share 2."},
    {"shamir_phrase_verify_success_title", "Verified"},
    {"sign_transaction", "Transaction Signing"},
    {"sign_transaction_desc", "Please Wait..."},
    {"single_backup_choose_backup_desc", "Select the preferred method for backing up your seed phrase."},
    {"single_backup_choose_backup_title", "Backup Options"},
    {"single_backup_learn_more_desc", "The Shamir Backup method provides a highly secure way to recover a seed phrase. It involves splitting the seed phrase into multiple fragments and specifying the required number of fragments needed to restore the phrase."},
    {"single_backup_learn_more_link", "Learn More"},
    {"single_backup_learn_more_qr_link", "https://keyst.one/b/3rd/shamir"},
    {"single_backup_learn_more_qr_title", "What is Shamir Backup?"},
    {"single_backup_learn_more_title", "What is Shamir Backup?"},
    {"single_backup_namewallet_desc", "Name your wallet and select an icon to make it easily distinguishable."},
    {"single_backup_namewallet_previntput", "Wallet Name"},
    {"single_backup_namewallet_previntput_2", "Pick an icon for your wallet"},
    {"single_backup_namewallet_title", "Customize Your Wallet"},
    {"single_backup_notice_desc1", "Having your seed phrase in someone else's possession grants them complete access to your assets."},
    {"single_backup_notice_desc2", "Ensure you're in a private and secure location, free from people or surveillance cameras, when writing down your seed phrase."},
    {"single_backup_notice_title", "Check Your Surroundings"},
    {"single_backup_phrase_regenerate", "Regenerate"},
    {"single_backup_phrase_regenerate_button_1", "Cancel"},
    {"single_backup_phrase_regenerate_button_2", "Regenerate"},
    {"single_backup_phrase_regenerate_desc", "Regenerate a set of Seed Phrase?"},
    {"single_backup_repeatpass_desc", "Double confirm the password you've entered."},
    {"single_backup_repeatpass_title", "Re-Enter Password"},
    {"single_backup_repeatpin_desc", "Double confirm the PIN code you've entered."},
    {"single_backup_repeatpin_error", "PIN code does not match"},
    {"single_backup_repeatpin_title", "Re-Enter PIN Code"},
    {"single_backup_setpass_desc", "The password will be used to unlock your wallet and authorize transactions."},
    {"single_backup_setpass_desc_1", "Set a strong password"},
    {"single_backup_setpass_error_1", "Password must be at least 6 characters"},
    {"single_backup_setpass_error_2", "Password does not match"},
    {"single_backup_setpass_title", "Set Password"},
    {"single_backup_setpin_desc", "The PIN code will be used to unlock your wallet and authorize transactions."},
    {"single_backup_setpin_title", "Set PIN Code"},
    {"single_backup_setpin_use_pass", "Use Password"},
    {"single_backup_setpin_use_pin", "Use PIN Code"},
    {"single_backup_shamir_desc", "An advanced procedure for securely storing your seed phrase."},
    {"single_backup_shamir_title", "Shamir Backup"},
    {"single_backup_single_phrase_desc", "12 or 24 seed phrase backup. Most commonly used method."},
    {"single_backup_single_phrase_title", "Standard Seed Phrase"},
    {"single_phrase_12words", "12 Words"},
    {"single_phrase_24words", "24 Words"},
    {"single_phrase_confirm_desc", "Select the words below in the correct order to validate your seed phrase."},
    {"single_phrase_confirm_title", "Confirm Your Seed"},
    {"single_phrase_desc", "Write down your seed phrase and store it in a secure location."},
    {"single_phrase_low_battery_desc", "The device needs a minimum of 20% battery life to continue the process"},
    {"single_phrase_low_battery_tilte", "Low Battery"},
    {"single_phrase_not_match_desc", "The seed phrase is incorrect. Please re-verify and try again."},
    {"single_phrase_not_match_title", "Seed Phrase Mismatch"},
    {"single_phrase_reset", "Reset"},
    {"single_phrase_title", "Backup Your Seed"},
    {"single_phrase_word_amount_select", "Seed Phrase Count"},
    {"support_link", "support@keyst.one"},
    {"switch_account", "Switch Account"},
    {"switch_address", "Switch Address"},
    {"system_settings_screen_lock_auto_lock", "Auto Lock"},
    {"system_settings_screen_lock_auto_lock_10mins", "10 minutes"},
    {"system_settings_screen_lock_auto_lock_15secs", "15 seconds"},
    {"system_settings_screen_lock_auto_lock_1min", "1 minute"},
    {"system_settings_screen_lock_auto_lock_30secs", "30 seconds"},
    {"system_settings_screen_lock_auto_lock_5mins", "5 minutes"},
    {"system_settings_screen_lock_auto_lock_title", "Timeout Duration"},
    {"system_settings_screen_lock_auto_power_off_12h", "12 hours"},
    {"system_settings_screen_lock_auto_power_off_1d", "1 day"},
    {"system_settings_screen_lock_auto_power_off_1h", "1 hour"},
    {"system_settings_screen_lock_auto_power_off_6h", "6 hours"},
    {"system_settings_screen_lock_auto_power_off_never", "Never"},
    {"system_settings_screen_lock_auto_power_off_title", "Auto Shutdown"},
    {"system_settings_screen_lock_auto_shutdown", "Auto Shutdown"},
    {"system_settings_screen_lock_brightness", "Brightness"},
    {"system_settings_screen_lock_title", "Display & Lock Screen"},
    {"system_settings_title", "System Settings"},
    {"system_settings_vabiration", "Vibration"},
    {"system_settings_wipe_device_generating_desc1", "Erasing Secure Element..."},
    {"system_settings_wipe_device_generating_desc2", "Do not power off your device while the installation process is underway"},
    {"system_settings_wipe_device_generating_title", "Resetting Device"},
    {"system_settings_wipe_device_wipe_alert_desc", "Please confirm that continuing will permanently delete all data, including wallets, on this device."},
    {"system_settings_wipe_device_wipe_alert_title", "Wipe Device"},
    {"system_settings_wipe_device_wipe_button", "Wipe Device Now"},
    {"system_settings_wipe_device_wipe_desc", "All data stored on this device, including all of your wallets, will be permanently deleted."},
    {"system_settings_wipe_device_wipe_end_text", "Wipe"},
    {"system_settings_wipe_device_wipe_fmt", "Wipe(%d)"},
    {"system_settings_wipe_device_wipe_start_text", "Wipe(5)"},
    {"system_settings_wipe_device_wipe_title", "Wipe Device"},
    {"transaction_parse_broadcast_message", "Broadcast Message"},
    {"transaction_parse_confirm_message", "Confirm Message"},
    {"transaction_parse_scan_by_software", "Scan the QR code with your software wallet"},
    {"try_again", "Try Again"},
    {"tx_details_btc_change_desc", "BTC transactions, based on the UTXO mechanism, allocate some assets to a change address during transfers. This address is generated by the software wallet for anonymity and privacy purposes. You don't need to worry since the change address belongs to your own account, and its amount will be used automatically in subsequent transactions."},
    {"tx_details_btc_change_link", "Learn More"},
    {"tx_details_btc_change_qr_link", "https://keyst.one/change"},
    {"tx_details_btc_change_qr_title", "What is a 'Change' address?"},
    {"tx_details_btc_change_title", "Change Address"},
    {"tx_details_eth_decoding_link", "Learn More"},
    {"tx_details_eth_decoding_qr_link", "https://keyst.one/t/3rd/ddt"},
    {"tx_details_eth_decoding_qr_title", "Decoding DeFi Transactions"},
    {"tx_details_general_tab_title1", "Overview"},
    {"tx_details_general_tab_title2", "Details"},
    {"unlock_device_attempts_left_plural_times_fmt", "Incorrect password, #F55831 %d# attampts left"},
    {"unlock_device_attempts_left_singular_times_fmt", "Incorrect password, #F55831 %d# attampt left"},
    {"unlock_device_button1", "PASSWORD"},
    {"unlock_device_button2", "PIN CODE"},
    {"unlock_device_button3", "FORGET"},
    {"unlock_device_error_attempts_exceed", "Attempt Limit Exceeded"},
    {"unlock_device_error_attempts_exceed_desc", "Device lock imminent. Please unlock to access the device."},
    {"unlock_device_error_btn_end_text", "Unlock Device"},
    {"unlock_device_error_btn_start_text", "Unlock Device (5s)"},
    {"unlock_device_error_btn_text_fmt", "Unlock Device (%ds)"},
    {"unlock_device_fingerprint_pin_device_locked_btn", "Wipe Device Now"},
    {"unlock_device_fingerprint_pin_device_locked_btn_fmt", "Wipe Device Now (%d)"},
    {"unlock_device_fingerprint_pin_device_locked_btn_start_text", "Wipe Device Now (15)"},
    {"unlock_device_fingerprint_pin_device_locked_desc", "All the data on this device will be erased after wiped"},
    {"unlock_device_fingerprint_pin_device_locked_title", "Device Locked"},
    {"unlock_device_fingerprint_pin_error1_desc", "Couldn’t verify fingerprint"},
    {"unlock_device_fingerprint_pin_error1_title", "Try Again"},
    {"unlock_device_fingerprint_pin_error2_title", "Use PIN or Fingerprint"},
    {"unlock_device_fingerprint_pin_error_max_desc", "Too many unsuccessful attempts. Please enter your passcode"},
    {"unlock_device_fingerprint_pin_error_max_title", "Enter PIN"},
    {"unlock_device_fingerprint_pin_title", "Use PIN or Fingerprint"},
    {"unlock_device_time_limited_error_max_desc", "Please unlock your device in 1 minute"},
    {"unlock_device_time_limited_error_max_desc_fmt", "Please unlock your device in %d minutes"},
    {"unlock_device_time_limited_error_max_title", "Device Unavailable"},
    {"unlock_device_title_fmt", "%s or %s"},
    {"unlock_device_use_fingerprint", "Use Fingerprint"},
    {"unlock_device_use_password", "Use Password"},
    {"unlock_device_use_pin", "Use PIN"},
    {"usb_connection_desc", "When disabled, the usb can only be used for charging battery"},
    {"usb_connection_subtitle", "Data transfer with USB"},
    {"usb_connection_title", "Connection"},
    {"verification_code_desc", "Enter this code on Keystone's official website to verify your device's security."},
    {"verification_code_failed_button", "Erase Data & Shut Down"},
    {"verification_code_failed_desc", "Your device may have been compromised, posing a risk to your sensitive data and digital assets.\nFor your safety, we recommend erasing all personal data and contacting KeystoneSupport team immediately for assistance."},
    {"verification_code_failed_link", "support@keyst.one"},
    {"verification_code_failed_title", "Unauthorized breach attempt detected!"},
    {"verification_code_title", "Verification Code"},
    {"verification_failed", "Failed"},
    {"verification_success", "Verified"},
    {"verify_cont1", "Please access the link stated below. Click the #F5870A Start Verification# button to initiate the process."},
    {"verify_cont1_link", "https://keyst.one/verify"},
    {"verify_cont2", "Scan the QR code generated on the website to obtain your device verification code."},
    {"verify_cont3", "Enter the code on the website to check whether the device has been compromised or not."},
    {"verify_desc", "This procedure is to verify that the Keystone device or firmware has not been tampered with."},
    {"verify_modal_desc", "Calculating Auth Code..."},
    {"verify_modal_title", "Verifying"},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_qr_title", "Verify Your Device"},
    {"verify_scan_qr_code", "Scan QR Code"},
    {"verify_title", "Verify Your Device"},
    {"wallet_setting_add_wallet", "Add Wallet"},
    {"wallet_setting_passcode", "Fingerprint & Passcode"},
    {"wallet_setting_passphrase", "Passphrase"},
    {"wallet_setting_seed_phrase", "Seed Phrase Check"},
    {"wallet_settings_add_info_desc1", "Keystone supports a maximum of #F5870A 3# different wallets."},
    {"wallet_settings_add_info_desc2", "You should set a #F5870A different passcode# for each wallet"},
    {"wallet_settings_add_info_desc3", "To switch wallets, unlock the device with the specific passcode associated with each one. The wallet list will not be displayed for enhanced security."},
    {"wallet_settings_add_info_title", "Notice"},
    {"wallet_settings_delete_button", "Delete Wallet"},
    {"wallet_settings_delete_confirm_button1", "Seed Phrase Check"},
    {"wallet_settings_delete_confirm_button2", "Confirm Deletion"},
    {"wallet_settings_delete_confirm_desc", "To safeguard your digital assets, it is advisable to verify the seed phrase before proceeding with its deletion."},
    {"wallet_settings_delete_confirm_title", "Delete Wallet?"},
    {"wallet_settings_delete_laoding_desc", "Erasing Secure Element..."},
    {"wallet_settings_delete_laoding_title", "Deleting"},
    {"wallet_settings_mid_btn", "Wallet Settings"},
    {"wipe_device", "Wipe Device"},
    {NULL, NULL} // End mark
};



static uint8_t en_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);
    uint32_t i = op_i(n); UNUSED(i);
    uint32_t v = op_v(n); UNUSED(v);

    if ((i == 1 && v == 0)) return LV_I18N_PLURAL_TYPE_ONE;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t en_lang = {
    .locale_name = "en",
    .singulars = en_singulars,

    .locale_plural_fn = en_plural_fn
};

const lv_i18n_language_pack_t lv_i18n_language_pack[] = {
    &en_lang,
    NULL // End mark
};

////////////////////////////////////////////////////////////////////////////////


// Internal state
static const lv_i18n_language_pack_t * current_lang_pack;
static const lv_i18n_lang_t * current_lang;


/**
 * Reset internal state. For testing.
 */
void __lv_i18n_reset(void)
{
    current_lang_pack = NULL;
    current_lang = NULL;
}

/**
 * Set the languages for internationalization
 * @param langs pointer to the array of languages. (Last element has to be `NULL`)
 */
int lv_i18n_init(const lv_i18n_language_pack_t * langs)
{
    if(langs == NULL) return -1;
    if(langs[0] == NULL) return -1;

    current_lang_pack = langs;
    current_lang = langs[0];     /*Automatically select the first language*/
    return 0;
}

/**
 * Change the localization (language)
 * @param l_name name of the translation locale to use. E.g. "en-GB"
 */
int lv_i18n_set_locale(const char * l_name)
{
    if(current_lang_pack == NULL) return -1;

    uint16_t i;

    for(i = 0; current_lang_pack[i] != NULL; i++) {
        // Found -> finish
        if(strcmp(current_lang_pack[i]->locale_name, l_name) == 0) {
            current_lang = current_lang_pack[i];
            return 0;
        }
    }

    return -1;
}


static const char * __lv_i18n_get_text_core(lv_i18n_phrase_t * trans, const char * msg_id)
{
    uint16_t i;
    for(i = 0; trans[i].msg_id != NULL; i++) {
        if(strcmp(trans[i].msg_id, msg_id) == 0) {
            /*The msg_id has found. Check the translation*/
            if(trans[i].translation) return trans[i].translation;
        }
    }

    return NULL;
}


/**
 * Get the translation from a message ID
 * @param msg_id message ID
 * @return the translation of `msg_id` on the set local
 */
const char * lv_i18n_get_text(const char * msg_id)
{
    if(current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;

    // Search in current locale
    if(lang->singulars != NULL) {
        txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
        if (txt != NULL) return txt;
    }

    // Try to fallback
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->singulars != NULL) {
        txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
        if (txt != NULL) return txt;
    }

    return msg_id;
}

/**
 * Get the translation from a message ID and apply the language's plural rule to get correct form
 * @param msg_id message ID
 * @param num an integer to select the correct plural form
 * @return the translation of `msg_id` on the set local
 */
const char * lv_i18n_get_text_plural(const char * msg_id, int32_t num)
{
    if(current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;
    lv_i18n_plural_type_t ptype;

    // Search in current locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
            if (txt != NULL) return txt;
        }
    }

    // Try to fallback
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
            if (txt != NULL) return txt;
        }
    }

    return msg_id;
}

/**
 * Get the name of the currently used locale.
 * @return name of the currently used locale. E.g. "en-GB"
 */
const char * lv_i18n_get_current_locale(void)
{
    if(!current_lang) return NULL;
    return current_lang->locale_name;
}

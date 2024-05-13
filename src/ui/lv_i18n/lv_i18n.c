#include "./lv_i18n.h"

////////////////////////////////////////////////////////////////////////////////
// Define plural operands
// http://unicode.org/reports/tr35/tr35-numbers.html#Operands

// Integer version, simplified

#define UNUSED(x) (void)(x)

static inline uint32_t op_n(int32_t val)
{
    return (uint32_t)(val < 0 ? -val : val);
}
static inline uint32_t op_i(uint32_t val)
{
    return val;
}
// always zero, when decimal part not exists.
static inline uint32_t op_v(uint32_t val)
{
    UNUSED(val);
    return 0;
}
static inline uint32_t op_w(uint32_t val)
{
    UNUSED(val);
    return 0;
}
static inline uint32_t op_f(uint32_t val)
{
    UNUSED(val);
    return 0;
}
static inline uint32_t op_t(uint32_t val)
{
    UNUSED(val);
    return 0;
}
static inline uint32_t op_e(uint32_t val)
{
    UNUSED(val);
    return 0;
}

static const lv_i18n_phrase_t en_singulars[] = {
    {"Address", "Address"},
    {"Approve", "Approve"},
    {"Attention", "Attention"},
    {"Cancel", "Cancel"},
    {"Confirm", "Confirm"},
    {"Continue", "Continue"},
    {"Delete", "Delete"},
    {"Details", "Details"},
    {"Done", "Done"},
    {"Export", "Export"},
    {"FORGET", "FORGET"},
    {"Failed", "Failed"},
    {"Import", "Import"},
    {"Keystone", "Keystone"},
    {"OK", "OK"},
    {"Overview", "Overview"},
    {"Passphrase", "Passphrase"},
    {"Path", "Path"},
    {"Pending", "Pending"},
    {"Policy", "Policy"},
    {"Quit", "Quit"},
    {"Regenerate", "Regenerate"},
    {"Reject", "Reject"},
    {"Restart", "Restart"},
    {"Restart_now", "Restart now"},
    {"Skip", "Skip"},
    {"Success", "Success"},
    {"Tutorial", "Tutorial"},
    {"Undo", "Undo"},
    {"Update", "Update"},
    {"Updating", "Updating"},
    {"Warning", "Warning"},
    {"about_info_battery_voltage", "Battery Voltage"},
    {"about_info_device_uid", "Device UID"},
    {"about_info_export_file_name", "File name:"},
    {"about_info_export_log", "Export System Log"},
    {"about_info_export_to_sdcard", "Export Log to MicroSD Card"},
    {"about_info_fingerprint_firmware_version", "Fingerprint Firmware Version"},
    {"about_info_firmware_version", "Firmware Version"},
    {"about_info_firmware_version_head", "Firmware"},
    {"about_info_result_export_failed", "Export Failed"},
    {"about_info_result_export_failed_desc_no_sdcard", "Please ensure you've inserted a MicroSD Card formatted in FAT32."},
    {"about_info_result_export_failed_desc_no_space", "Please make sure your MicroSD card has enough memory."},
    {"about_info_result_export_successful", "Export Successful"},
    {"about_info_result_export_successful_desc", "Your system log has been successfully exported to the MicroSD Card."},
    {"about_info_serial_number", "Serial Number"},
    {"about_info_verify_checksum_desc", "Please check if the above information matches the webpage. If it does not match, it means that your firmware may have been tampered with. Please stop using it immediately."},
    {"about_info_verify_checksum_text", "Checksum"},
    {"about_info_verify_checksum_title", "Checksum"},
    {"about_info_verify_firmware_desc", " This is an advanced feature for developers to verify that the firmware running on your Keystone device is consistent with the one we open-sourced."},
    {"about_info_verify_firmware_step1", "Go to Keystone's open-sourced GitHub repo and follow the instructions to build your firmware and get the checksum."},
    {"about_info_verify_firmware_step2", "Click the #F5870A Checksum# button next to the firmware download."},
    {"about_info_verify_firmware_step3", "Tap the #F5870A Show Checksum# button below and compare the info shown on the webpage and your device."},
    {"about_info_verify_source_code_title", "Verify Source Code"},
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
    {"about_terms_law", "Governing Law and Dispute Resolution"},
    {"about_terms_law_desc", "The Terms are governed by Hong Kong SAR laws, and any dispute must be filed within one year."},
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
    {"account_head", "Account"},
    {"auto_shutdown", "Auto Shutdown"},
    {"auto_shutdown_20", "Auto Shutdown"},
    {"beta_version_notice_desc", "The current firmware is a beta version and may contain unknown bugs or risks. It is intended only for testing purposes."},
    {"beta_version_notice_title", "Beta Version"},
    {"calculat_modal_title", "Calculating"},
    {"change_entropy", "Change Entropy"},
    {"change_entropy_desc", "Select the Entropy you'd like to use for creating wallets"},
    {"change_entropy_dice_desc", "Create your seed Phrase with random numbers generated by rolling dice."},
    {"change_entropy_dice_detail_desc_1", "You need to roll at least #F5870A 50 times# to generate a seed phrase, and it is recommended roll more than #F5870A 99 times# for sufficient randomness."},
    {"change_entropy_dice_detail_desc_2", "We recommended you use casino dice to increase the entropy of each dice throw."},
    {"change_entropy_dice_rolls", "Dice Rolls"},
    {"change_entropy_dice_rolls_subtitle", "Use Dice Rolls as entropy"},
    {"change_entropy_system", "System"},
    {"change_entropy_system_desc", "Create your seed phrase from the keystone SEs' true random number generator."},
    {"change_entropy_system_subtitle", "Use Keystone SEs' entropy"},
    {"change_passcode_mid_btn", "Enter Passcode"},
    {"change_passcode_reset_success_desc", "Your passcode has been reset successfully."},
    {"change_passcode_reset_success_title", "Reset Successful"},
    {"change_passcode_reset_title", "Resetting, Keep Device ON"},
    {"change_passcode_warning_desc", "If forgotten, you'll have to verify the seed phrase for this wallet to reset the passcode."},
    {"change_passcode_warning_title", "Remember your Passcode"},
    {"confirm_transaction", "Confirm Transaction"},
    {"connect_block_link", "https://keyst.one/t/3rd/block"},
    {"connect_block_title", "BlockWallet (Extension)"},
    {"connect_bw_link", "https://keyst.one/t/3rd/bw"},
    {"connect_bw_multisig_link", "https://keyst.one/t/3rd/Multisig/bw"},
    {"connect_bw_title", "BlueWallet (Mobile)"},
    {"connect_eternl_link", "https://keyst.one/t/3rd/eternl"},
    {"connect_eternl_title", "Eternl (Web)"},
    {"connect_fewcha_link", "https://keyst.one/t/3rd/fewcha"},
    {"connect_fewcha_title", "Fewcha (Extension)"},
    {"connect_head", "Connect"},
    {"connect_imtoken_link", "https://keyst.one/t/3rd/imtoken"},
    {"connect_imtoken_title", "imToken (Mobile)"},
    {"connect_keplr_link", "https://keyst.one/t/3rd/keplr"},
    {"connect_keplr_title", "Keplr (Extension)"},
    {"connect_mm_link", "https://keyst.one/t/3rd/mm"},
    {"connect_mm_link2", "https://keyst.one/t/3rd/mmm"},
    {"connect_mm_title", "MetaMask (Extension)"},
    {"connect_mm_title2", "MetaMask (Mobile)"},
    {"connect_nunchuk_multisig_link", "https://keyst.one/t/3rd/Multisig/nunchuk"},
    {"connect_nunchuk_title", "Nunchuk (Mobile)"},
    {"connect_okx_link", "https://keyst.one/t/3rd/okx"},
    {"connect_okx_link2", "https://keyst.one/t/3rd/okxm"},
    {"connect_okx_title", "OKX Wallet (Extension)"},
    {"connect_okx_title2", "OKX Wallet (Mobile)"},
    {"connect_petra_link", "https://keyst.one/t/3rd/petra"},
    {"connect_petra_title", "Petra (Extension)"},
    {"connect_rabby_link", "https://keyst.one/t/3rd/rabby"},
    {"connect_rabby_title", "Rabby (Extension)"},
    {"connect_safe_link", "https://keyst.one/t/3rd/safe"},
    {"connect_safe_link2", "https://keyst.one/t/3rd/safem"},
    {"connect_safe_title", "Safe (Web)"},
    {"connect_safe_title2", "Safe (Mobile)"},
    {"connect_solflare_link", "https://keyst.one/t/3rd/solflare"},
    {"connect_solflare_title", "Solflare"},
    {"connect_backpack_link", "https://keyst.one/t/3rd/backpack"},
    {"connect_backpack_title", "Backpack"},
    {"connect_sparrow_link", "https://keyst.one/t/3rd/sparrow"},
    {"connect_sparrow_multisig_link", "https://keyst.one/t/3rd/Multisig/sparrow"},
    {"connect_sparrow_title", "Sparrow"},
    {"connect_specter_link", "https://keyst.one/t/3rd/specter"},
    {"connect_specter_title", "Specter"},
    {"connect_sushi_link", "https://keyst.one/t/3rd/sushi"},
    {"connect_sushi_title", "SushiSwap"},
    {"connect_typhon_link", "https://keyst.one/t/3rd/typhon"},
    {"connect_typhon_title", "Typhon (Web)"},
    {"connect_unisat_link", "https://keyst.one/t/3rd/unisat"},
    {"connect_unisat_title", "UniSat"},
    {"connect_wallet_ada_step1", "Select accounts you’d like to import on your %s wallet"},
    {"connect_wallet_ada_step2", "Scan the QR code via your Keystone"},
    {"connect_wallet_ada_step3", "Approve the request to generate a new QR code on your Keystone"},
    {"connect_wallet_ada_step4", "Scan the QR code via %s wallet"},
    {"connect_wallet_choose_wallet", "Choose Wallet"},
    {"connect_wallet_desc", "Visit the link below on your computer or mobile device for a guide on syncing Keystone with your software wallet."},
    {"connect_wallet_instruction", "Follow the instructions below:"},
    {"connect_wallet_key_request_fmt", "%s Wallet wants to get your public key on your Keystone:"},
    {"connect_wallet_keystone_hint", "Select networks you’d like to manage in the software wallet"},
    {"connect_wallet_scan", "Scan the QR code with your software wallet"},
    {"connect_wallet_select_network", "Select Network"},
    {"connect_wallet_select_network_hint", "Select the networks you'd like to display within the software wallet."},
    {"connect_wallet_supported_networks", "Supported Networks"},
    {"connect_wallet_title", "Connect Wallet"},
    {"connect_wallet_upgrade_hint", "Please upgrade to the latest version for access to expanded software wallet compatibility."},
    {"connect_wallet_xpub_qrformat", "QR Code Format"},
    {"connect_wallet_xpub_script_format", "Script Format"},
    {"connect_xrp_toolkit_link", "https://keyst.one/t/3rd/xrptoolkit"},
    {"connect_xrp_toolkit_title", "XRP Toolkit (Web)"},
    {"connect_yearn_link", "https://keyst.one/t/3rd/yearn"},
    {"connect_yearn_title", "Yearn"},
    {"connect_zapper_link", "https://keyst.one/t/3rd/zapper"},
    {"connect_zapper_title", "Zapper"},
    {"create_multi_wallet_cancel_desc", "If you cancel, any confirmed information will be lost."},
    {"create_multi_wallet_cancel_title", "Cancel Wallet Creation?"},
    {"create_multi_wallet_co_sign", "Co-Sign"},
    {"create_multi_wallet_co_sign_policy", "Co-Sign Policy"},
    {"create_multi_wallet_co_signers", "Co-Signers"},
    {"create_multi_wallet_co_signers_desc", "This is the xPub key of your multi-signature wallet. You can also view this xPub on the home page of multisig wallet at any time."},
    {"create_multi_wallet_co_signers_desc_fmt", "Import the xPub of the %d co-signer to construct a multi-signature wallet."},
    {"create_multi_wallet_eg_desc", "Recommended. Nunchunk and Sparrow wallet usually use this format"},
    {"create_multi_wallet_import_xpub_qr", "Import via QR Code"},
    {"create_multi_wallet_import_xpub_sdcard", "Import Via MicoSD Card"},
    {"create_multi_wallet_import_xpub_sdcard_title", "MicoSD Card"},
    {"create_multi_wallet_import_xpub_title", "xPub"},
    {"create_multi_wallet_multi_xpub", "Multisig xPub"},
    {"create_multi_wallet_name_desc", "Name your multi-sigature wallet"},
    {"create_multi_wallet_select_format", "Select the format you'd like to use for your multisig wallet"},
    {"create_multi_wallet_signers", "Signers"},
    {"create_multi_wallet_xpub_duplicated_desc", "The imported xpub file is duplicated"},
    {"create_multi_wallet_xpub_duplicated_title", "Duplicate Xpub"},
    {"create_wallet_generating_title", "Creating Wallet, Keep Device ON"},
    {"derivation_path_address", "Select the derivation path"},
    {"derivation_path_address_eg", "Addresses eg."},
    {"derivation_path_btc_1_desc", "Modern format, lower fees, starts with \"bc1.\""},
    {"derivation_path_btc_2_desc", "Legacy & SegWit compatible, medium fees, starts with \"3.\""},
    {"derivation_path_btc_3_desc", "Original format, higher fees, starts with \"1.\""},
    {"derivation_path_btc_4_desc", "Taproot: Enhanced privacy and efficiency, starts with \"bc1p.\""},
    {"derivation_path_btc_test_net_1_desc", "Modern format, lower fees, starts with \"tb1q.\""},
    {"derivation_path_btc_test_net_2_desc", "Legacy & SegWit compatible, medium fees, starts with \"2.\""},
    {"derivation_path_btc_test_net_3_desc", "Original format, higher fees, starts with \"m.\" or \"n.\""},
    {"derivation_path_btc_test_net_4_desc", "Taproot: Enhanced privacy and efficiency, starts with \"tb1p.\""},
    {"derivation_path_change", "Change Derivation Path"},
    {"derivation_path_eth_ledger_legacy_desc", "Older Ledger format, less common."},
    {"derivation_path_eth_ledger_live_desc", "Ledger-specific, optimized for Ledger device."},
    {"derivation_path_eth_standard_desc", "Default, follows BIP44, widely used."},
    {"derivation_path_near_standard_desc", "Recommended. Widely adopted across numerous NEAR wallets."},
    {"derivation_path_select_btc", "Select the address type you’d like to use for Bitcoin"},
    {"derivation_path_select_eth", "Select the derivation path you’d like to use for Ethereum"},
    {"derivation_path_select_near", "Select the derivation path you'd like to use for Near"},
    {"derivation_path_select_sol", "Select the derivation path you’d like to use for Solana"},
    {"device_info_title", "Device Info"},
    {"device_setting_about_title", "About"},
    {"device_setting_connection_desc", "USB / MicroSD Card"},
    {"device_setting_mid_btn", "Device Settings"},
    {"device_setting_system_setting_desc", "Language / Screen / Reset..."},
    {"device_setting_system_setting_title", "System Settings"},
    {"device_setting_wallet_setting_desc", "Name / Passcode / Passphrase..."},
    {"device_setting_wallet_setting_title", "Wallet Settings"},
    {"device_settings_connection_desc1", "When disabled, the usb can only be used for charging battery"},
    {"dice_roll_cancel_desc", "If you cancel, any numbers you entered will be lost."},
    {"dice_roll_cancel_title", "Cancel Dice Rolls Generation?"},
    {"dice_roll_error_label", "Lack of randomness"},
    {"dice_roll_hint_label", "At least 50 times"},
    {"dice_roll_max_limit_label", "You've reached the max limits"},
    {"dice_rolls_entropy_hint", "Dice rolls as entropy"},
    {"enter_passcode", "Enter Passcode"},
    {"error_box_duplicated_seed_phrase", "Duplicate Seed Phrase"},
    {"error_box_duplicated_seed_phrase_desc", "This phrase you typed is already used in a wallet account, please import another set of seed phrase."},
    {"error_box_firmware_not_detected", "Firmware Not Detected"},
    {"error_box_firmware_not_detected_desc", "Please ensure that your MicroSD card is formatted in FAT32 and contains the firmware 'keystone3.bin'."},
    {"error_box_invalid_seed_phrase", "Invalid Seed Phrase"},
    {"error_box_invalid_seed_phrase_desc", "The seed phrase you've entered is invalid. Please re-verify your backup and try again."},
    {"error_box_low_power", "Low Battery"},
    {"error_box_low_power_desc", "The device needs a minimum of 20% battery life to continue the process."},
    {"error_box_mnemonic_not_match_wallet", "Seed Phrase Mismatch"},
    {"error_box_mnemonic_not_match_wallet_desc", "The seed phrase is incorrect. Please re-verify and try again. "},
    {"error_unknown_error", "Unknown Error"},
    {"error_unknown_error_desc", "The device has encountered an unknown issue and is currently unavailable. To resolve this, please wipe and restart the device. If the problem persists, please contact our support team."},
    {"fingerprint_add", "Add Fingerprint"},
    {"fingerprint_add_btn", "+ Add Fingerprint"},
    {"fingerprint_add_desc", "Position your finger on the sensor and lift it up once you sense the vibration."},
    {"fingerprint_add_failed", "Failed to Add"},
    {"fingerprint_add_failed_duplicate", "Duplicate finger, please use a different finger"},
    {"fingerprint_add_failed_partial", "Partial fingerprint detected, please try again"},
    {"fingerprint_add_failed_use_another", "Failed to add fingerprint, please use another finger and try again"},
    {"fingerprint_add_overlaps_too_much", "Duplicate area, please adjust your finger's position slightly"},
    {"fingerprint_add_password", "Now we need you to enter your passcode to encrypt and store the fingerprint."},
    {"fingerprint_add_poor_qualtiy", "Fingerprint unclear, please restart process and use different finger to try again"},
    {"fingerprint_add_success", "Add Successfully"},
    {"fingerprint_add_too_wet", "Moisture detected. Please wipe your finger and try again"},
    {"fingerprint_nth", "Finger %d"},
    {"fingerprint_nth_remove_desc", "Do you really want to delete Finger %d?"},
    {"fingerprint_nth_remove_title", "Remove Fingerprint?"},
    {"fingerprint_nth_subtitle", "Finger %d"},
    {"fingerprint_passcode_fingerprint_setting", "Fingerprint Settings"},
    {"fingerprint_passcode_reset_passcode", "Reset Passcode"},
    {"fingerprint_remove", "Remove Fingerprint"},
    {"fingerprint_remove_confirm", "Remove"},
    {"fingerprint_sign_tx", "Sign Transactions"},
    {"fingerprint_up_to_3", "You can add up to 3 fingerprints"},
    {"firmware_update_btc_only_button_i_know", "I know"},
    {"firmware_update_btc_only_warning_desc", "You are upgrading from multi-coin firmware to BTC-only firmware. After the upgrade, #F5870A all the wallets on this device will# #F5870A only support BTC currency#, and this process is irreversible. Please confirm that you understand the risks involved. If you have downloaded the wrong firmware, please cancel this progress."},
    {"firmware_update_deny_desc", "You need to unlock your device to upgrade the firmware version."},
    {"firmware_update_deny_input_password", "Enter Password"},
    {"firmware_update_deny_title", "Unlock Device Required"},
    {"firmware_update_desc", "To unlock the newest features, please update your firmware to the latest version."},
    {"firmware_update_desc1", "Ensure that your Keystone has at least 20% battery life remaining."},
    {"firmware_update_desc2", "Go to Keystone's firmware update page using your computer or smartphone."},
    {"firmware_update_no_upgradable_firmware_desc", "Your device firmware version is higher than or equal to the one on your microSD card."},
    {"firmware_update_no_upgradable_firmware_title", "No Upgradable Firmware Detected"},
    {"firmware_update_sd_checksum_desc", "#F5870A Show Checksum#"},
    {"firmware_update_sd_checksum_done", "Checksum:\n"},
    {"firmware_update_sd_checksum_fmt", "#F5870A Show Checksum(%d%%)#"},
    {"firmware_update_sd_checksum_fmt_version", "#F5870A Show Checksum(v%s)#"},
    {"firmware_update_sd_checksum_notice", "This is an optional feature to further enhance security. Compare the following checksum with the checksum of your download package on the official website, ensure that they are consistent."},
    {"firmware_update_sd_copying_desc", "Do not remove the MicroSD Card while the update is underway."},
    {"firmware_update_sd_copying_title", "Starting Update"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Transfer the firmware file (keystone3.bin) to your Keystone using a MicroSD card formatted in FAT32."},
    {"firmware_update_sd_desc4", "Tap the #F5870A Update# button below to initiate the process."},
    {"firmware_update_sd_dialog_desc", "A new firmware version is available. Do you want to update your device's firmware to the new version?"},
    {"firmware_update_sd_dialog_head", "Available"},
    {"firmware_update_sd_dialog_title", "Update Available"},
    {"firmware_update_sd_failed_access_desc", "Please ensure that you have properly inserted the MicroSD Card."},
    {"firmware_update_sd_failed_access_title", "MicroSD Card Not Detected"},
    {"firmware_update_sd_title", "MicroSD Update"},
    {"firmware_update_title", "Firmware Update"},
    {"firmware_update_updating_desc", "Takes around 5 mins"},
    {"firmware_update_usb_connect_info_desc", "Once connected, the external device will gain permission to transfer data to your Keystone."},
    {"firmware_update_usb_connect_info_title", "Connect to This Device?"},
    {"firmware_update_usb_desc3", "Connect your Keystone to your computer using a USB-C cable."},
    {"firmware_update_usb_desc4", "Click the #F5870A Install Update# button on the webpage and follow the instructions to install the latest firmware."},
    {"firmware_update_usb_desc5", "Do not unplug the USB cable while the installation process is underway."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "Firmware Update"},
    {"firmware_update_usb_title", "USB Update"},
    {"firmware_update_usb_title2", "Caution"},
    {"firmware_update_usb_updating_hint", "Do not disconnect the USB cable during the installation process."},
    {"firmware_update_verify_firmware_qr_link", "KeystoneHQ/keystone3-firmware/docs/verify.md"},
    {"firmware_update_via_sd", "Via MicroSD Card"},
    {"firmware_update_via_usb", "Via USB"},
    {"forget_password_cancel", "Cancel Password Reset?"},
    {"forgot_password_reset_passcode_intro_desc", "Verify the seed phrase associated with this wallet to reset the passcode.Resetting your password may erase your wallet or multisig wallet data"},
    {"forgot_password_reset_passcode_intro_text", "Forgot passcode?"},
    {"forgot_password_reset_passcode_intro_title", "Forgot passcode?"},
    {"generating_qr_codes", "Generating QR Codes"},
    {"got_it", "Got It"},
    {"got_it_fmt", "Got It (%d)"},
    {"got_it_start", "Got It (5)"},
    {"home_button_receive", "RECEIVE"},
    {"home_button_scan", "SCAN"},
    {"home_manage_assets", "Manage Assets"},
    {"home_more_connect_wallet", "Connect Software Wallet"},
    {"home_more_sign_by_sdcard", "Sign from Micro SD card"},
    {"home_select_coin_count_fmt", "#F5870A %d#/%d"},
    {"home_upgrade_hint", "Please upgrade to the latest version for access to expanded crypto compatibility."},
    {"illustrate_supplement", ""},
    {"import_multi_wallet_file_limit_desc", "Files larger than 256KB cannot be displayed"},
    {"import_multi_wallet_info_no_config_file", "No Wallet Files"},
    {"import_multi_wallet_info_title", "Wallet Info"},
    {"import_multi_wallet_success_title", "Import Success"},
    {"import_multi_wallet_via_camera", "Via Camera"},
    {"import_multi_wallet_via_micro_title", "MicroSD Card"},
    {"import_wallet_choose_method", "Choose Import Method"},
    {"import_wallet_duplicated_share_desc", "You’ve already checked this share, please use another share to continue."},
    {"import_wallet_invalid_phrase_desc", "The phrase you typed is invalid. Please check your backup and try again"},
    {"import_wallet_phrase_clear_btn", "Clear"},
    {"import_wallet_phrase_desc", "Enter your seed phrase in the blanks provided below."},
    {"import_wallet_phrase_title", "Import Your Seed"},
    {"import_wallet_shamir_backup", "Shamir Backup"},
    {"import_wallet_shamir_backup_desc", "You'll need a couple of seed phrase\nshares to recover your wallet"},
    {"import_wallet_share_success_desc", "This share of your seed phrase matches your wallet."},
    {"import_wallet_single_backup_desc", "Recover your wallet with the specific seed phrase. Please complete the setup process in one go to avoid any security risks."},
    {"import_wallet_single_phrase", "Single Secret Phrase"},
    {"import_wallet_single_phrase_desc", "You'll need a 12/18/24 seed phrase\nto recover your wallet."},
    {"import_wallet_ssb_cancel_desc", "Upon cancellation, you will be required to re-enter all the Shares again."},
    {"import_wallet_ssb_cancel_title", "Cancel Import Wallet?"},
    {"import_wallet_ssb_desc_fmt", "Enter your  #F5870A %d#-word seed phrase for Share #F5870A %d# in the blanks provided below."},
    {"import_wallet_ssb_incorrect_title", "Duplicate Share"},
    {"import_wallet_ssb_notbelong_desc", "The Share you've entered doesn't correspond to this backup. Please re-verify your backup and try again."},
    {"import_wallet_ssb_repeat_desc", "The Share entered has already been previously inputted. Please submit a different Share."},
    {"import_wallet_ssb_step_fmt", "%d of %d"},
    {"import_wallet_ssb_title_fmt", "Share #F5870A %d#"},
    {"language_desc", "Select your language"},
    {"language_little_title", "Language"},
    {"language_option", "English"},
    {"language_title", "Language"},
    {"learn_more", "Learn More"},
    {"manage_import_wallet_notice_desc1", "Please compare the wallet information with your co-signers or other wallets to ensure that all information is consistent. "},
    {"manage_import_wallet_notice_desc2", "It is crucial to securely backup your wallet configuration; failure to do so could result in irreversible loss of assets. If the information has been modified or is inconsistent, this may also lead to asset loss. Always verify and safeguard your wallet details to prevent any discrepancies."},
    {"manage_import_wallet_notice_title", "Check the Wallet Info"},
    {"manage_import_wallet_passphrase_error_desc", "Unable to import multisig wallet into the Passphrase wallet. Please exit Passphrase mode and retry."},
    {"manage_import_wallet_passphrase_error_title", "Import Error"},
    {"manage_multi_wallet_add_limit_desc", "Add up to 4 multisig wallets, including test net and main net."},
    {"manage_multi_wallet_add_limit_title", "Multisig Limit Reached"},
    {"manage_multi_wallet_add_scan_limit_title", "A maximum of 4 multisig wallets are supported; unable to add more multisig wallets."},
    {"manage_multi_wallet_delete_current_wallet_desc", "Current Wallet can not be deleted"},
    {"manage_multi_wallet_delete_desc", "This wallet can not be restored after the deletion"},
    {"manage_multi_wallet_detail_title", "Wallet Details"},
    {"manage_multi_wallet_export_attention_desc1", "When exporting or re-importing your multi-signature wallet, ensure all data remains consistent for you and your co-signers."},
    {"manage_multi_wallet_export_attention_desc2", "It's critical to sync and verify details like MFP, derivation path, and xPub with every co-signer. Accurate data ensures your wallet's seamless operation and security."},
    {"manage_multi_wallet_export_attention_title", "Attention Required"},
    {"manage_multi_wallet_export_config", "Export MultiSig Wallet"},
    {"manage_multi_wallet_export_title", "Export Wallet"},
    {"manage_multi_wallet_set_default", "Set as Current Wallet"},
    {"manage_wallet_confirm_address_desc", "verify that the 'Receive' address matches across all parties' devices or software wallets."},
    {"manage_wallet_confirm_address_title", "Verify Address"},
    {"manage_wallet_passphrase_error_limit", "Passphrase wallet can't add multisig"},
    {"multi_wallet_no_file_notice", "Please make sure the file is named with English letters and located in the root path"},
    {"multi_wallet_no_psbt_file", "No PSBT Files"},
    {"multisig_connect_wallet_notice", "You are using multisig wallet"},
    {"multisig_decoding_qr_link", "https://keyst.one/t/3rd/Multisig/import"},
    {"multisig_decoding_qr_title", "How to import multisig wallet"},
    {"multisig_export_to_sdcard", "Export Wallet to MicroSD Card"},
    {"multisig_export_to_sdcard_failed", "Exported Failed"},
    {"multisig_export_to_sdcard_failed_desc", "Something went wrong. Please try again."},
    {"multisig_export_to_sdcard_success", "Exported Successfully"},
    {"multisig_export_to_sdcard_success_desc", "Your multisig wallet has been exported to MicoSD card successfully."},
    {"multisig_import_success_hint", "Invite co-signers to scan the QR code to import multisig wallet or scan this QR code with software wallet to finish connection"},
    {"multisig_import_wallet_exist", "Wallet Already Exists"},
    {"multisig_import_wallet_exist_desc", "The multisig wallet you selected already exists on the current device, please import another wallet."},
    {"multisig_import_wallet_invalid", "Invalid Wallet Config"},
    {"multisig_import_wallet_invalid_desc", "The current wallet configuration is incorrect. Please carefully check whether the MFP, Path, and xPub information is accurate."},
    {"multisig_import_xpub_error_title", "Invalid File"},
    {"multisig_scan_multisig_notice", "Scan the multisig wallet QR code to import"},
    {"multisig_signature_export_to_sdcard", "Export Transaction to MicroSD Card"},
    {"multisig_signature_export_to_sdcard_success_desc", "Your signed transaction has been exported to MicoSD card successfully."},
    {"multisig_signature_hint_1", "Please ask the co-signers to scan the QR code and sign."},
    {"mutlisig_transaction_already_signed", "Signed Transaction"},
    {"mutlisig_transaction_already_signed_desc", "Please reselect the transaction file."},
    {"not_now", "Not Now"},
    {"passphrase_access_switch_desc", "Create a passphrase shortcut for device boot-up"},
    {"passphrase_access_switch_title", "Passphrase Quick Access"},
    {"passphrase_add_password", "Now we need you to enter your passcode to setup passphrase wallet."},
    {"passphrase_enter_input", "Input passphrase"},
    {"passphrase_enter_repeat", "Confirm passphrase"},
    {"passphrase_error_not_match", "Passphrase mismatch"},
    {"passphrase_error_too_long", "input length cannot exceed 128 characters"},
    {"passphrase_learn_more_desc1", "A passphrase offers an extra level of security to your current seed phrase."},
    {"passphrase_learn_more_desc2", "Each unique passphrase generates a different wallet."},
    {"passphrase_learn_more_desc3", "To recover your wallet, both the passphrase and seed phrase are required."},
    {"passphrase_learn_more_desc4", "Forgetting the passphrase can result in the loss of access to your digital assets."},
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
    {"password_score_good", "Good"},
    {"password_score_normal", "Normal"},
    {"password_score_weak", "Weak"},
    {"pin_code", "PIN CODE"},
    {"pin_label", "PIN"},
    {"please_enter_passcode", "Please Enter Passcode"},
    {"power_off", "Power Off"},
    {"prepare_wallet_first_step", "New chains support detected"},
    {"prepare_wallet_hint", "Preparing Wallet"},
    {"prepare_wallet_second_step", "Setting up for wallet..."},
    {"prepare_wallet_third_step", "Generating extended public key..."},
    {"public_key", "Public Key"},
    {"purpose_desc", "Create a new wallet or import an existing wallet with the seed phrase."},
    {"purpose_import_wallet", "Import Wallet"},
    {"purpose_new_wallet", "Create Wallet"},
    {"purpose_title", "New Wallet"},
    {"receive_ada_enterprise_address", "Address (Not Delegated)"},
    {"receive_ada_more_t_desc1", "On the Cardano blockchain, we provide 24 accounts, and each account can generate numerous addresses for your use. You can easily switch between the accounts you need."},
    {"receive_ada_more_t_desc2", "1. Payment & Stake Keys: In Cardano, every account has a Payment Key for regular ADA transactions (sending and receiving) and a Stake Key for staking and receiving rewards.\n2. Base Address: A Base Address is derived from both the Payment Key and Stake Key. It can be used for both regular transactions and staking. Also known as \"External Addresses (Delegated).\"\n3. Enterprise Address: This address only contains the Payment Key and is used solely for regular transactions, not for staking. It's designed for \"business\" scenarios that don't involve staking, like exchanges. Also known as \"External Addresses (Not Delegated).\"\n4. Stake & Reward Addresses: The Stake Key is used for staking, and the associated Stake Address is also called a Reward Address, used to receive staking rewards."},
    {"receive_ada_more_t_title1", "Multiple Cardano Accounts on Keystone"},
    {"receive_ada_more_t_title2", "Key Concepts in Cardano's ADA Addresses"},
    {"receive_ada_show_address_detail", "Show Address Detail"},
    {"receive_ada_stake_address", "Reward Address"},
    {"receive_btc_address_type", "Address Type"},
    {"receive_btc_alert_desc", "This address is exclusively for BTC transactions only. Sending other types of digital assets to this address will result in their loss."},
    {"receive_btc_extended_public_key", "Extended Public Key"},
    {"receive_btc_more_address_settings", "Address Settings"},
    {"receive_btc_more_export_xpub", "Export xPub"},
    {"receive_btc_more_t_desc1", "Bitcoin (BTC) uses three address formats for receiving funds:\n1. Native SegWit is the most efficient and secure Bitcoin address format. It provides cost savings and improved security compared to other traditional address formats, typically starting with \"bc1\"\n2. Legacy format is one of the earliest versions of Bitcoin, typically starting with \"1\"\n3. Nested SegWit is a solution designed to facilitate the transition to Native SegWit in a smooth manner, typically starting with \"3\""},
    {"receive_btc_more_t_desc2", "Yes, the three distinct Bitcoin address formats can be used for transferring funds among each other. However, it’s important to keep in mind the following aspects:\n1. Differing transaction fees: The choice of address format can influence transaction fees, with Native SegWit addresses generally having lower fees.\n2. Wallet and exchange compatibility: Make sure that the wallet or exchange you are using supports your chosen address format. Some wallets may only be compatible with specific address formats."},
    {"receive_btc_more_t_desc3", "1. Privacy: Reusing addresses increases transaction traceability, endangering privacy. New addresses help maintain transaction privacy.\n2. Transaction Efficiency: Multiple UTXOs linked to one address can raise costs for consolidation, impacting wallet efficiency.\n3. Security: Repeated address use heightens the risk of private key exposure, potentially leading to losses if compromised.\nIn short, not reusing addresses safeguards privacy, optimizes transactions, and reduces security risks within the UTXO model."},
    {"receive_btc_more_t_title1", "What are the three different address formats for BTC?"},
    {"receive_btc_more_t_title2", "Can the three different address formats be used to transfer funds to each other?"},
    {"receive_btc_more_t_title3", "Benefits of not reusing addresses"},
    {"receive_btc_receive_change_address_limit", "Can't exceed 999,999,999"},
    {"receive_btc_receive_change_address_title", "Go to"},
    {"receive_coin_fmt", "Receive %s"},
    {"receive_coin_hint_fmt", "This address is only for %s, other digital assets sent to this address will be lost."},
    {"receive_eth_alert_desc", "This address is only for ETH and EVM ERC-20 tokens, other digital assets sent to this address will be lost."},
    {"receive_eth_more_derivation_path_bip", "BIP 44 Standard"},
    {"receive_eth_more_derivation_path_desc", "Recommend. Most commonly used in many software wallets."},
    {"receive_eth_more_derivation_path_desc2", "Select this path for Ledger Live asset management."},
    {"receive_eth_more_derivation_path_desc3", "Select this path for Ledger Legacy asset management."},
    {"receive_eth_more_derivation_path_ledger_legacy", "Ledger Legacy"},
    {"receive_eth_more_derivation_path_ledger_live", "Ledger Live"},
    {"receive_eth_more_derivation_path_title2", "Accounts eg:"},
    {"receive_eth_more_t_desc1", "1. Standard Path: This path is widely employed by numerous software wallets for address generation. Examples of such wallets encompass MetaMask, Rabby, BitKeep, and Core Wallet.\n2. Ledger Live: Choose this path if you intend to import a seed phrase from Ledger Live. Please note that Keystone supports synchronizing only the initial 10 addresses using this format.\n3. Ledger Legacy: Transition to this path if you manage your digital assets with Ledger Legacy"},
    {"receive_eth_more_t_title1", "Exploring Differences: Standard, Ledger Live, and Legacy Derivation Paths in Ethereum"},
    {"receive_eth_receive_main_title", "Receive ETH"},
    {"receive_generate_new_address", "Generate New Address"},
    {"receive_more_t_qr_link", "https://keyst.one/t/3rd/faq"},
    {"receive_sol_more_t_base_path", "Account-based Path"},
    {"receive_sol_more_t_desc1", "1. Account-based Path: Widely adopted across numerous Solana wallets. An example of such a wallet is Solflare.\n2. Single Account Path: Choose this path if you intend to import a seed phrase from Sollet / MathWallet. Please note that Keystone supports synchronizing only the initial 10 addresses using this format.\n3. Sub-account Path: Transition to this path if you manage your digital assets with software wallets like Phantom / Exodus."},
    {"receive_sol_more_t_single_path", "Single Account Path"},
    {"receive_sol_more_t_sub_path", "Sub-account Path"},
    {"receive_sol_more_t_title1", "Exploring Solana Path Options"},
    {"receive_trx_hint", "This address is only for TRX, TRC-20 tokens and TRC-10 tokens, other digital assets sent to this address will be lost."},
    {"repeat_passcode_desc", "Double confirm the PIN code you've entered."},
    {"repeat_passcode_title", "Re-Enter PIN Code"},
    {"rust_error_bitcoin_no_my_inputs", "Incongruent Transaction"},
    {"rust_error_bitcoin_no_my_inputs_desc", "The transaction doesn't belong to the current wallet. Please check the transaction information."},
    {"rust_error_bitcoin_not_my_multisig_wallet", "Invalid Multisig Wallet Config"},
    {"rust_error_bitcoin_not_my_multisig_wallet_desc", "This multisig wallet config doesn't belong to the current wallet. Please check the multisig wallet config information."},
    {"scan_qr_code_error_invalid_file_desc", "The date of file not recognized. Please try again."},
    {"scan_qr_code_error_invalid_qrcode", "Invalid QR Code"},
    {"scan_qr_code_error_invalid_qrcode_desc", "QR code data not recognized. Please try again."},
    {"scan_qr_code_error_invalid_wallet_file", "Invalid Wallet File"},
    {"scan_qr_code_error_invalid_wallet_file_desc", "Please ensure that you choose or scan correct multisig wallet."},
    {"scan_qr_code_invalid_b_desc", "The transaction does not belong to the current wallet. Please ensure you are using the correct wallet to authorize the transaction."},
    {"scan_qr_code_invalid_b_title", "Unpermitted Authorization"},
    {"scan_qr_code_invalid_c_desc", "Keystone cannot facilitate transaction signing for the current address path. Please try again using a different address path."},
    {"scan_qr_code_invalid_c_title", "Unsupported Path"},
    {"scan_qr_code_sign_fingerprint_verify_fingerprint", "Verify Fingerprint"},
    {"scan_qr_code_sign_fingerprint_verify_fingerprint_failed", "Verify Failed. Please try Again!"},
    {"scan_qr_code_sign_unsigned_content_fingerprint_failed_desc", "Verification failed. Please try again!"},
    {"scan_qr_code_sign_unsigned_content_fingerprint_failed_desc2", "If the verification fails again, the fingerprint signing for transactions will automatically be disabled."},
    {"scan_qr_code_sign_unsigned_content_frame", "Swipe to confirm"},
    {"scan_qr_code_signing_desc", "Signing"},
    {"sdcard_format_confirm", "Format"},
    {"sdcard_format_desc", "Your MicroSD Card will be formatted to FAT32, erasing all files. Please back up essential files before formatting."},
    {"sdcard_format_failed_desc", "Formatting failed, replace the MicroSD Card or format it on the computer."},
    {"sdcard_format_failed_title", "Formatting Failure"},
    {"sdcard_format_subtitle", "Format MicroSD Card"},
    {"sdcard_format_success_desc", "How about - The MicroSD Card has been successfully formatted to FAT32."},
    {"sdcard_format_success_title", "Formatting Complete"},
    {"sdcard_format_text", "Format MicroSD Card"},
    {"sdcard_formating", "Formatting"},
    {"sdcard_formating_desc", "Do not remove the MicroSD card while formatting is in progress."},
    {"seed_check_passphrase_notice", "You are presently using a passphrase-protected wallet. Prior to proceeding with the seed phrase verification process, please restart your device without entering the passphrase."},
    {"seed_check_passphrase_title", "Disable Passphrase Wallet"},
    {"seed_check_share_phrase", "Shamir Backup"},
    {"seed_check_share_phrase_title", "Enter your seed phrase to verify if it matches your current wallet."},
    {"seed_check_single_phrase", "Standard Seed Phrase"},
    {"seed_check_single_phrase_title", "Enter Your Seed"},
    {"seed_check_verify_match_desc", "Your seed has been validated and successfully verified."},
    {"seed_check_verify_match_title", "Verification Successful"},
    {"seed_check_verify_not_match_title", "Verification Failed"},
    {"seed_check_wait_verify", "Verifying"},
    {"seed_check_word_select", "Seed Phrase Word Count"},
    {"self_destruction_desc", "A physical attack has been detected, leading to the complete erasure of all data stored on this device. As a result, this device is now inoperable."},
    {"self_destruction_hint", "Contact us"},
    {"self_destruction_title", "Device No Longer Usable"},
    {"set_passcode_desc", "This PIN code will be used to unlock your wallet and authorize transactions."},
    {"shamir_backup", "Shamir Backup"},
    {"shamir_phrase_backup_desc", "Write down your Share #F5870A 1# phrase and keep it properly."},
    {"shamir_phrase_cancel_create_desc", "If you cancel, any confirmed Shares will be lost."},
    {"shamir_phrase_cancel_create_title", "Quit Wallet Setup?"},
    {"shamir_phrase_confirm_desc", "Select the words below in the correct order of your Share 1 phrase to validate your seed phrase."},
    {"shamir_phrase_custodian_desc", "Please confirm you are the custodian of the Share #F5870A 1#"},
    {"shamir_phrase_custodian_title", "Share Share #F5870A 1#"},
    {"shamir_phrase_number", "Number of Shares"},
    {"shamir_phrase_share_backup_notice_fmt", "Write down your Share #F5870A %d# phrase and keep it properly."},
    {"shamir_phrase_share_confirm_notice_fmt", "Select the words below in the correct order of your Share #F5870A %d# phrase to validate your seed phrase."},
    {"shamir_phrase_share_notice_fmt", "Please confirm you are the custodian of the Share #F5870A %d#"},
    {"shamir_phrase_share_number_fmt", "Share #F5870A %d#/%d"},
    {"shamir_phrase_threold", "Threshold"},
    {"shamir_phrase_verify_success_desc1", "The seed phrase for this Share has been validated, please proceed to the next Share."},
    {"shamir_phrase_verify_success_desc2", "Tap the button below and hand the Keystone over to the custodian of Share 2."},
    {"shamir_phrase_verify_success_title", "Verified"},
    {"show_checksum", "Show checksum"},
    {"sign_transaction", "Transaction Signing"},
    {"sign_transaction_desc", "Please Wait..."},
    {"single_backup_choose_backup_desc", "Select the preferred method for backing up your seed phrase."},
    {"single_backup_choose_backup_title", "Backup Options"},
    {"single_backup_learn_more_desc", "The Shamir Backup method provides a highly secure way to recover a seed phrase. It involves splitting the seed phrase into multiple fragments and specifying the required number of fragments needed to restore the phrase."},
    {"single_backup_learn_more_qr_link", "https://keyst.one/b/3rd/shamir"},
    {"single_backup_learn_more_title", "What is Shamir Backup?"},
    {"single_backup_namewallet_desc", "Name your wallet and select an icon to make it easily distinguishable."},
    {"single_backup_namewallet_previnput", "Wallet Name"},
    {"single_backup_namewallet_previnput_2", "Pick an icon for your wallet"},
    {"single_backup_namewallet_title", "Name Your Wallet"},
    {"single_backup_notice_desc1", "Never share your seed phrase with anyone else, as it grants access to your assets."},
    {"single_backup_notice_desc2", "Ensure there are no onlookers or cameras when recording your seed phrase."},
    {"single_backup_notice_title", "Check Your Surroundings"},
    {"single_backup_phrase_regenerate_desc", "Regenerate a new set of Seed Phrase?"},
    {"single_backup_repeatpass_desc", "Double confirm the password you've entered."},
    {"single_backup_repeatpass_title", "Re-Enter Password"},
    {"single_backup_repeatpin_error", "PIN code does not match"},
    {"single_backup_setpass_desc", "The password will be used to unlock your wallet and authorize transactions."},
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
    {"single_phrase_confirm_desc", "Select the words below in the correct order to validate your seed phrase."},
    {"single_phrase_confirm_title", "Confirm Your Seed"},
    {"single_phrase_desc", "Write down your seed phrase and store it in a secure location."},
    {"single_phrase_low_battery_desc", "The device needs a minimum of 20% battery life to continue the process"},
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
    {"system_settings_screen_lock_brightness", "Brightness"},
    {"system_settings_screen_lock_title", "Display & Lock Screen"},
    {"system_settings_vabiration", "Vibration"},
    {"system_settings_wipe_device_generating_desc1", "Deleting all data..."},
    {"system_settings_wipe_device_generating_desc2", "Please do not turn off your device during the wiping process."},
    {"system_settings_wipe_device_generating_title", "Resetting Device"},
    {"system_settings_wipe_device_wipe_alert_desc", "Please confirm that continuing will permanently delete all data, including wallets, on this device."},
    {"system_settings_wipe_device_wipe_button", "Wipe Device Now"},
    {"system_settings_wipe_device_wipe_desc", "By proceeding, all data on this device, including all your wallets, will be permanently deleted."},
    {"system_settings_wipe_device_wipe_end_text", "Wipe"},
    {"system_settings_wipe_device_wipe_fmt", "Wipe(%d)"},
    {"system_settings_wipe_device_wipe_start_text", "Wipe(5)"},
    {"system_settings_wipe_device_wipe_title", "Wipe Device"},
    {"transaction_parse_broadcast_message", "Broadcast Message"},
    {"transaction_parse_confirm_message", "Confirm Message"},
    {"transaction_parse_scan_by_software", "Scan the QR code with your software wallet"},
    {"try_again", "Try Again"},
    {"tutorial_change_entropy_desc1", "Entropy is a measure of randomness. In cryptography, it ensures that things like passwords or keys are unpredictable, making them more secure."},
    {"tutorial_change_entropy_desc2", "Computers aren't great at being truly random. Dice rolls provide a physical, unpredictable source of randomness. By using them, you enhance the security of cryptographic processes, making it harder for someone to predict or crack your codes."},
    {"tutorial_change_entropy_title1", "What is entropy?"},
    {"tutorial_change_entropy_title2", "Why use Dice Rolls as your entropy?"},
    {"tx_details_btc_change_desc", "BTC transactions, based on the UTXO mechanism, allocate some assets to a change address during transfers. This address is generated by the software wallet for anonymity and privacy purposes. You don't need to worry since the change address belongs to your own account, and its amount will be used automatically in subsequent transactions."},
    {"tx_details_btc_change_qr_link", "https://keyst.one/change"},
    {"tx_details_btc_change_qr_title", "What is a 'Change' address?"},
    {"tx_details_eth_decoding_qr_link", "https://keyst.one/t/3rd/ddt"},
    {"tx_details_eth_decoding_qr_title", "Decoding DeFi Transactions"},
    {"unknown_transaction_desc", "The data within this transaction currently cannot be decoded."},
    {"unknown_transaction_title", "Transaction Details Unavailable"},
    {"unlock_device", "Unlock Device"},
    {"unlock_device_attempts_left_plural_times_fmt", "Incorrect password, %d attempts left"},
    {"unlock_device_attempts_left_singular_times_fmt", "Incorrect password, %d attempt left"},
    {"unlock_device_error_attempts_exceed", "Attempt Limit Exceeded"},
    {"unlock_device_error_attempts_exceed_desc", "Device lock imminent. Please unlock to access the device."},
    {"unlock_device_error_btn_start_text", "Unlock Device (5s)"},
    {"unlock_device_error_btn_text_fmt", "Unlock Device (%ds)"},
    {"unlock_device_fingerprint_pin_device_locked_btn_fmt", "Wipe Device Now (%d)"},
    {"unlock_device_fingerprint_pin_device_locked_btn_start_text", "Wipe Device Now (15)"},
    {"unlock_device_fingerprint_pin_device_locked_desc", "All the data on this device will be erased after wiped"},
    {"unlock_device_fingerprint_pin_device_locked_title", "Device Locked"},
    {"unlock_device_fingerprint_pin_title", "Use PIN or Fingerprint"},
    {"unlock_device_time_limited_error_max_desc", "Please unlock your device in #F55831 1# minute"},
    {"unlock_device_time_limited_error_max_desc_fmt", "Please unlock your device in #F55831 %d# minutes"},
    {"unlock_device_time_limited_error_max_title", "Device Unavailable"},
    {"unlock_device_time_limited_error_max_warning_fmt", "Please unlock your device in #F55831 %d# minutes. One more incorrect attempt, your device will be locked."},
    {"unlock_device_title_passcode_fingerprint", "Use Password or Use Fingerprint"},
    {"unlock_device_title_pin_fingerprint", "Use PIN or Use Fingerprint"},
    {"unlock_device_use_fingerprint", "Use Fingerprint"},
    {"unlock_device_use_password", "Use Password"},
    {"unlock_device_use_pin", "Use PIN"},
    {"update_success", "Update Successful"},
    {"usb_connection_desc", "When enabled, the USB port can only be used for battery charging purposes."},
    {"usb_connection_subtitle", "Air-Gap Mode"},
    {"usb_connection_title", "Connection"},
    {"usb_transport_connect_rabby", "Rabby Wallet wants to connect your Keystone via USB"},
    {"usb_transport_connect_wallet", "Your software wallet wants to connect to your device via USB"},
    {"usb_transport_connection_request", "Connection Request"},
    {"usb_transport_mismatched_wallet_title", "Mismatched Wallet"},
    {"usb_transport_sign_completed_subtitle", "View transaction details at your software wallet"},
    {"usb_transport_sign_completed_title", "Signature Completed"},
    {"usb_transport_sign_failed_title", "Signature Failed"},
    {"usb_transport_sign_unkown_error_message", "Unable to recognize data information"},
    {"usb_transport_sign_unkown_error_title", "Unknown Error"},
    {"verification_code_desc", "Enter this code on Keystone's official website to verify your device's security."},
    {"verification_code_failed_desc", "Your device may have been compromised, posing a risk to your sensitive data and digital assets.For your safety, we recommend erasing all personal data and contacting Keystone Support team immediately for assistance."},
    {"verification_code_failed_title", "Unauthorized breach attempt detected!"},
    {"verification_code_title", "Verification Code"},
    {"verification_success", "Verified"},
    {"verify_cont1", "Visit our website and click #F5870A Start Verification# button."},
    {"verify_cont2", "Scan the QR code displayed on the website to get your device verification code."},
    {"verify_cont3", "Enter the code on the website to check for any potential tampering with your device."},
    {"verify_desc", "This process ensures the integrity of your Keystone device and firmware."},
    {"verify_firmware", "Verify Firmware"},
    {"verify_modal_desc", "Calculating Auth Code..."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "Scan QR Code"},
    {"verify_title", "Verify Your Device"},
    {"verify_title_text", "Verify Your Device"},
    {"wallet_phrase_12words", "12 Words"},
    {"wallet_phrase_18words", "18 Words"},
    {"wallet_phrase_20words", "20 Words"},
    {"wallet_phrase_24words", "24 Words"},
    {"wallet_phrase_33words", "33 Words"},
    {"wallet_profile_add_multi_wallet", "Add MultiSig Wallet"},
    {"wallet_profile_create_multi_wallet", "Create MultiSig Wallet"},
    {"wallet_profile_current_default_desc", "Currently Wallet"},
    {"wallet_profile_default_desc", "Current Wallet"},
    {"wallet_profile_export_title", "Export (Ext) Public Key"},
    {"wallet_profile_export_to_sdcard_title", "Export xPub to MicroSD Card"},
    {"wallet_profile_import_multi_wallet", "Import MultiSig Wallet"},
    {"wallet_profile_import_multi_wallet_desc", "Choose the method you'd like to import multi-signature wallets"},
    {"wallet_profile_mid_btn", "Wallet Profile"},
    {"wallet_profile_multi_sign_title", "MultiSig Wallet"},
    {"wallet_profile_multi_wallet_show_xpub", "Show/Export xPub"},
    {"wallet_profile_network_main", "MainNet"},
    {"wallet_profile_network_test", "Testnet"},
    {"wallet_profile_network_title", "Network"},
    {"wallet_profile_no_multi_wallet_notice", "No MultiSig wallet yet"},
    {"wallet_profile_single_sign_title", "Single-Sig Wallet"},
    {"wallet_profile_single_wallet_title", "Singlesig Wallet"},
    {"wallet_setting_add_wallet", "+ Add Wallet"},
    {"wallet_setting_add_wallet_confirm", "I Understand"},
    {"wallet_setting_add_wallet_limit", "Add Limited"},
    {"wallet_setting_add_wallet_limit_desc", "You can only add up to a maximum of 3 wallets. Please delete other wallets before adding a new wallet."},
    {"wallet_setting_add_wallet_notice", "Hardware wallet is for personal use only. Do not share to avoid asset risks."},
    {"wallet_setting_passcode", "Fingerprint & Passcode"},
    {"wallet_setting_seed_phrase", "Seed Phrase Check"},
    {"wallet_setting_stop_add_fingerprint", "Cancel This Process?"},
    {"wallet_setting_stop_add_fingerprint_desc", "Fingerprint Not Saved. Upon cancellation, you will be required to re-enter your fingerprint."},
    {"wallet_settings_add_info_desc1", "Keystone supports a maximum of #F5870A 3# different wallets."},
    {"wallet_settings_add_info_desc2", "You should set a #F5870A different passcode# for each wallet"},
    {"wallet_settings_add_info_desc3", "To switch wallets, unlock the device with the specific passcode associated with each one. The wallet list will not be displayed for enhanced security."},
    {"wallet_settings_add_info_title", "Notice"},
    {"wallet_settings_delete_button", "Delete Wallet"},
    {"wallet_settings_delete_confirm_button2", "Confirm Deletion"},
    {"wallet_settings_delete_confirm_desc", "To safeguard your digital assets, it is advisable to verify the seed phrase before proceeding with its deletion."},
    {"wallet_settings_delete_confirm_title", "Delete Wallet?"},
    {"wallet_settings_delete_laoding_title", "Deleting"},
    {"wallet_settings_mid_btn", "Wallet Settings"},
    {"wipe_device", "Wipe Device"},
    {"write_se_desc", "Writing Secure Element..."},
    {NULL, NULL} // End mark
};

static uint8_t en_plural_fn(int32_t num)
{
    uint32_t n = op_n(num);
    UNUSED(n);
    uint32_t i = op_i(n);
    UNUSED(i);
    uint32_t v = op_v(n);
    UNUSED(v);

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
    if (langs == NULL) return -1;
    if (langs[0] == NULL) return -1;

    current_lang_pack = langs;
    current_lang = langs[0];     /*Automatically select the first language*/
    return 0;
}

/**
 * Sugar for simplified `lv_i18n_init` call
 */
int lv_i18n_init_default(void)
{
    return lv_i18n_init(lv_i18n_language_pack);
}

/**
 * Change the localization (language)
 * @param l_name name of the translation locale to use. E.g. "en-GB"
 */
int lv_i18n_set_locale(const char * l_name)
{
    if (current_lang_pack == NULL) return -1;

    uint16_t i;

    for (i = 0; current_lang_pack[i] != NULL; i++) {
        // Found -> finish
        if (strcmp(current_lang_pack[i]->locale_name, l_name) == 0) {
            current_lang = current_lang_pack[i];
            return 0;
        }
    }

    return -1;
}

static const char * __lv_i18n_get_text_core(const lv_i18n_phrase_t * trans, const char * msg_id)
{
    uint16_t i;
    for (i = 0; trans[i].msg_id != NULL; i++) {
        if (strcmp(trans[i].msg_id, msg_id) == 0) {
            /*The msg_id has been found. Check the translation*/
            if (trans[i].translation) return trans[i].translation;
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
    if (current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;

    // Search in current locale
    if (lang->singulars != NULL) {
        txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
        if (txt != NULL) return txt;
    }

    // Try to fallback
    if (lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if (lang->singulars != NULL) {
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
    if (current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;
    lv_i18n_plural_type_t ptype;

    // Search in current locale
    if (lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if (lang->plurals[ptype] != NULL) {
            txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
            if (txt != NULL) return txt;
        }
    }

    // Try to fallback
    if (lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if (lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if (lang->plurals[ptype] != NULL) {
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
    if (!current_lang) return NULL;
    return current_lang->locale_name;
}

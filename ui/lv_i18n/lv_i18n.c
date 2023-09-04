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

static lv_i18n_phrase_t en_singulars[] = {
    {"change_passcode_mid_btn", "Enter Passcode"},
    {"change_passcode_reset_desc", "Writing Secure Element..."},
    {"change_passcode_reset_success_desc", "Your passcode has been reset successfully."},
    {"change_passcode_reset_success_title", "Reset Successful"},
    {"change_passcode_reset_title", "Resetting, Keep Screen ON"},
    {"change_passcode_warning_desc", "If you forget it, you’ll have to verify the seed phrase of this wallet to reset the passcode."},
    {"change_passcode_warning_title", "Remember your Passcode"},
    {"connect_block_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_block_link", "https://keyst.one/t/3rd/block"},
    {"connect_block_qr_link", "https://keyst.one/t/3rd/block"},
    {"connect_block_qr_title", "BlockWallet"},
    {"connect_block_t", "Tutorial"},
    {"connect_block_title", "BlockWallet"},
    {"connect_bw_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_bw_link", "https://keyst.one/t/3rd/bw"},
    {"connect_bw_qr_link", "https://keyst.one/t/3rd/bw"},
    {"connect_bw_qr_title", "BlueWallet"},
    {"connect_bw_t", "Tutorial"},
    {"connect_bw_title", "BlueWallet"},
    {"connect_keplr_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_keplr_link", "https://keyst.one/t/3rd/keplr"},
    {"connect_keplr_qr_link", "https://keyst.one/t/3rd/keplr"},
    {"connect_keplr_qr_title", "Keplr (Extension)"},
    {"connect_keplr_t", "Tutorial"},
    {"connect_keplr_title", "Keplr (Extension)"},
    {"connect_keyst_app_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_keyst_app_link", "https://keyst.one/t/3rd/keystone"},
    {"connect_keyst_app_qr_link", "https://keyst.one/t/3rd/keystone"},
    {"connect_keyst_app_qr_title", "Keystone Companion App"},
    {"connect_keyst_app_t", "Tutorial"},
    {"connect_keyst_app_title", "Keystone Companion App"},
    {"connect_mm_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_mm_link", "https://keyst.one/t/3rd/mm"},
    {"connect_mm_link2", "https://keyst.one/t/3rd/mmm"},
    {"connect_mm_qr_link", "https://keyst.one/t/3rd/mm"},
    {"connect_mm_qr_link2", "https://keyst.one/t/3rd/mmm"},
    {"connect_mm_qr_title", "MetaMask"},
    {"connect_mm_qr_title2", "MetaMask Mobile"},
    {"connect_mm_t", "Tutorial"},
    {"connect_mm_title", "MetaMask"},
    {"connect_mm_title2", "MetaMask Mobile"},
    {"connect_okx_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_okx_link", "https://keyst.one/t/3rd/okx"},
    {"connect_okx_link2", "https://keyst.one/t/3rd/okxm"},
    {"connect_okx_qr_link", "https://keyst.one/t/3rd/okx"},
    {"connect_okx_qr_link2", "https://keyst.one/t/3rd/okxm"},
    {"connect_okx_qr_title", "OKX Wallet (Extension)"},
    {"connect_okx_qr_title2", "OKX Wallet (Mobile)"},
    {"connect_okx_t", "Tutorial"},
    {"connect_okx_title", "OKX Wallet (Extension)"},
    {"connect_okx_title2", "OKX Wallet (Mobile)"},
    {"connect_rabby_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_rabby_link", "https://keyst.one/t/3rd/rabby"},
    {"connect_rabby_qr_link", "https://keyst.one/t/3rd/rabby"},
    {"connect_rabby_qr_title", "Rabby"},
    {"connect_rabby_t", "Tutorial"},
    {"connect_rabby_title", "Rabby"},
    {"connect_safe_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_safe_link", "https://keyst.one/t/3rd/safe"},
    {"connect_safe_link2", "https://keyst.one/t/3rd/safem"},
    {"connect_safe_qr_link", "https://keyst.one/t/3rd/safe"},
    {"connect_safe_qr_link2", "https://keyst.one/t/3rd/safem"},
    {"connect_safe_qr_title", "Safe"},
    {"connect_safe_qr_title2", "Safe Mobile"},
    {"connect_safe_t", "Tutorial"},
    {"connect_safe_title", "Safe"},
    {"connect_safe_title2", "Safe Mobile"},
    {"connect_sushi_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_sushi_link", "https://keyst.one/t/3rd/sushi"},
    {"connect_sushi_qr_link", "https://keyst.one/t/3rd/sushi"},
    {"connect_sushi_qr_title", "SushiSwap"},
    {"connect_sushi_t", "Tutorial"},
    {"connect_sushi_title", "SushiSwap"},
    {"connect_wallet_desc", "Scan the QR code with your software wallet"},
    {"connect_wallet_title", "Connect Wallet"},
    {"connect_wallet_xpub_addresstype", "Address Type"},
    {"connect_wallet_xpub_qrformat", "QR Code Format"},
    {"connect_yearn_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_yearn_link", "https://keyst.one/t/3rd/yearn"},
    {"connect_yearn_qr_link", "https://keyst.one/t/3rd/yearn"},
    {"connect_yearn_qr_title", "Yearn"},
    {"connect_yearn_t", "Tutorial"},
    {"connect_yearn_title", "Yearn"},
    {"connect_zapper_desc", "Please use your laptop or mobile to open the link below for detailed tutorials on how to use Keystone with certain chain specific companion software wallet."},
    {"connect_zapper_link", "https://keyst.one/t/3rd/zapper"},
    {"connect_zapper_qr_link", "https://keyst.one/t/3rd/zapper"},
    {"connect_zapper_qr_title", "Zapper"},
    {"connect_zapper_t", "Tutorial"},
    {"connect_zapper_title", "Zapper"},
    {"create_wallet_generating_desc", "Writing Secure Element..."},
    {"create_wallet_generating_title", "Creating Wallet, Keep Screen ON"},
    {"device_setting_about_desc", "M-1.2.0"},
    {"device_setting_about_title", "About"},
    {"device_setting_connection_desc", "USB / Bluetooth / SD Card..."},
    {"device_setting_connection_title", "Connection"},
    {"device_setting_mid_btn", "Device Settings"},
    {"device_setting_system_setting_desc", "Language / Screen / Reset..."},
    {"device_setting_system_setting_title", "System Settings"},
    {"device_setting_wallet_setting_desc", "Name / Passcode / Passphrase..."},
    {"device_setting_wallet_setting_title", "Wallet Settings"},
    {"fingerprint_passcode_fingerprint_setting", "Fingerprint Settings"},
    {"fingerprint_passcode_mid_btn", "Fingerprint & Passcode"},
    {"fingerprint_passcode_reset_passcode", "Reset Passcode"},
    {"firmware_update_desc", "To take advantage of the latest features, make sure to update your firmware to the most recent version."},
    {"firmware_update_sd_copying_desc", "The copying process of the latest firmware from the MicroSD card may take 15 to 45 seconds."},
    {"firmware_update_sd_copying_title", "Copying"},
    {"firmware_update_sd_desc1", "Ensure that your Keystone has at least 20% battery life remaining."},
    {"firmware_update_sd_desc2", "Head over to"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Transfer the firmware file (keystone3.bin) onto your Keystone using MicroSD card (formatted in FAT32)."},
    {"firmware_update_sd_desc4", "Tap the #F5870A Update# button below."},
    {"firmware_update_sd_dialog_desc", "A new firmware version is available. Do you want to update your device's firmware to version v1.3.0?"},
    {"firmware_update_sd_dialog_title", "Update Available"},
    {"firmware_update_sd_failed_access_desc", "The SD card is not recognized. The current device requires formatting the SD card for re-recognition."},
    {"firmware_update_sd_failed_access_title", "firmware_update_sd_updating_desc"},
    {"firmware_update_sd_not_detected_desc", "Please ensure that you have inserted a microSD card formatted in FAT32 that contains the firmware."},
    {"firmware_update_sd_not_detected_title", "Firmware Not Detected"},
    {"firmware_update_sd_title", "Update via MicroSD"},
    {"firmware_update_sd_updating_desc", "Takes around 5 seconds"},
    {"firmware_update_sd_updating_title", "Updating"},
    {"firmware_update_title", "Firmware Update"},
    {"firmware_update_usb_connect_info_desc", "Once connected, the external device will have the necessary authorization to transfer data to your Keystone."},
    {"firmware_update_usb_connect_info_title", "Connect to This Device?"},
    {"firmware_update_usb_desc1", "Ensure that your Keystone has at least 20% battery life remaining."},
    {"firmware_update_usb_desc2", "Navigate to Keystone's firmware update page using your computer or mobile."},
    {"firmware_update_usb_desc2_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_desc3", "Connect Keystone to your computer with a USB-C cable."},
    {"firmware_update_usb_desc4", "Hit the #F5870A Install Update# button on the webpage and follow the instructions to install the latest firmware."},
    {"firmware_update_usb_desc5", "Do not unplug the USB cable while the installation process is underway."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "Firmware Update"},
    {"firmware_update_usb_title", "Update via USB"},
    {"firmware_update_usb_title2", "#F5870A Caution#"},
    {"firmware_update_usb_updating_desc", "Takes around 5 seconds"},
    {"firmware_update_usb_updating_hint", "Do not unplug the USB cable while the installation process is underway."},
    {"firmware_update_usb_updating_title", "Updating"},
    {"home_more_connect_wallet", "Connect Software Wallet"},
    {"home_more_device_setting", "Device Settings"},
    {"import_wallet_invalid_phrase_desc", "The phrase you typed is invalid. Please check your backup and try again"},
    {"import_wallet_invalid_phrase_title", "Invalid Seed Phrase"},
    {"import_wallet_phrase_12words", "12 Words"},
    {"import_wallet_phrase_18words", "18 Words"},
    {"import_wallet_phrase_20words", "20 Words"},
    {"import_wallet_phrase_24words", "24 Words"},
    {"import_wallet_phrase_33words", "33 Words"},
    {"import_wallet_phrase_clear_btn", "Clear"},
    {"import_wallet_phrase_desc", "Write down your seed phrase in the blanks below"},
    {"import_wallet_phrase_title", "Import Your Phrase"},
    {"import_wallet_phrase_words_title", "Phrase Words Amount"},
    {"import_wallet_ssb_cancel_desc", "You will be required to re-enter all the shares after canceling."},
    {"import_wallet_ssb_cancel_title", "Cancel Import Wallet?"},
    {"import_wallet_ssb_desc", "Write down your 20-words seed phrase of share #1 in the blanks below"},
    {"import_wallet_ssb_incorrect_title", "Incorrect Share"},
    {"import_wallet_ssb_notbelong_desc", "The share you entered is not belong to this backup. Please check your backup and try again."},
    {"import_wallet_ssb_repeat_desc", "Share already entered, please enter a different share."},
    {"import_wallet_ssb_title", "share"},
    {"language_desc", "Select your language"},
    {"language_option1", "English"},
    {"language_option2", "简体中文"},
    {"language_option3", "Русский язык"},
    {"language_option4", "Español"},
    {"language_option5", "한국인"},
    {"language_title", "Language"},
    {"low_battery_pop_up_desc", "For the process to continue, ensure that the device has a minimum of 20% battery power."},
    {"low_battery_pop_up_title", "Low Battery"},
    {"passphrase_access_switch_desc", "Display a shortcut entry for Passphrase when powered on"},
    {"passphrase_access_switch_title", "Passphrase Quick Access"},
    {"passphrase_enter_input", "Input passphrase"},
    {"passphrase_enter_passcode", "Passphrase"},
    {"passphrase_enter_repeat", "Repeat passphrase"},
    {"passphrase_learn_more_desc1", "The passphrase is an additional layer of security on top of your backup."},
    {"passphrase_learn_more_desc2", "Entering a different passphrase will always generate a different wallet."},
    {"passphrase_learn_more_desc3", "To restore your wallet you need both the passphrase and backup."},
    {"passphrase_learn_more_desc4", "If you forget your passphrase, you can no longer access your coins."},
    {"passphrase_learn_more_link", "Learn More"},
    {"passphrase_learn_more_qr_link", "https://keyst.one/t/3rd/passphrase"},
    {"passphrase_learn_more_qr_title", "What is Passphrase?"},
    {"passphrase_learn_more_title", "What is Passphrase?"},
    {"purpose_desc", "Create a new wallet or import a wallet from the existing seed phrase"},
    {"purpose_import_wallet", "Import Wallet"},
    {"purpose_new_wallet", "Create Wallet"},
    {"purpose_title", "New Wallet"},
    {"receive_btc_more_t", "Tutorial"},
    {"receive_btc_more_t_desc1", "Bitcoin (BTC) uses three address formats for receiving funds:\r\n1. Native SegWit is the most efficient and secure Bitcoin address format. It provides cost savings and improved security compared to other traditional address formats,typically starting with \"bc1\"\r\n2. Legacy address format is one of the earliest versions of Bitcoin, typically starting with \"1\"\r\n3. Nested SegWit is a solution designed to facilitate the transition to Native SegWit in a smooth manner, typically starting with \"3\""},
    {"receive_btc_more_t_desc2", "Yes, the three distinct Bitcoin address formats can be used for transferring funds among each other. However, it’s important to keep in mind the following aspects:\r\n1. Differing transaction fees: The choice of address format can influence transaction fees, with Native SegWit addresses generally having lower fees.\r\n2. Wallet and exchange compatibility: Make sure that the wallet or exchange you are using supports your chosen address format. Some wallets may only be compatible with specific address formats."},
    {"receive_btc_more_t_desc3", "1. Privacy: Reusing addresses increases transaction traceability, endangering privacy. New addresses help maintain transaction privacy.\r\n2. Transaction Efficiency: Multiple UTXOs linked to one address can raise costs for consolidation, impacting wallet efficiency.\r\n3. Security: Repeated address use heightens the risk of private key exposure, potentially leading to losses if compromised.\r\nIn short, not reusing addresses safeguards privacy, optimizes transactions, and reduces security risks within the UTXO model."},
    {"receive_btc_more_t_link1", "Learn More"},
    {"receive_btc_more_t_link2", "Learn More"},
    {"receive_btc_more_t_link3", "Learn More"},
    {"receive_btc_more_t_qr_link1", "https://keyst.one/t/3rd/faq"},
    {"receive_btc_more_t_qr_link2", "https://keyst.one/t/3rd/faq"},
    {"receive_btc_more_t_qr_link3", "https://keyst.one/t/3rd/faq"},
    {"receive_btc_more_t_qr_title1", "What are the three different address formats for BTC?"},
    {"receive_btc_more_t_qr_title2", "Can the three different address formats be used to transfer funds to each other?"},
    {"receive_btc_more_t_qr_title3", "Advantages of Avoiding Address Reuse"},
    {"receive_btc_more_t_title1", "What are the three different address formats for BTC?"},
    {"receive_btc_more_t_title2", "Can the three different address formats be used to transfer funds to each other?"},
    {"receive_btc_more_t_title3", "Advantages of Avoiding Address Reuse"},
    {"receive_eth_more_t", "Tutorial"},
    {"receive_eth_more_t_desc1", "1. Standard Path: This path is widely employed by numerous software wallets for address generation. Examples of such wallets encompass MetaMask, Rabby, BitKeep, and Core Wallet.\r\n2. Ledger Live: Choose this path if you intend to import a seed phrase from Ledger Live. Please note that Keystone supports synchronizing only the initial 10 addresses using this format.\r\n3. Ledger Legacy: Transition to this path if you manage your digital assets with Ledger Legacy"},
    {"receive_eth_more_t_link1", "Learn More"},
    {"receive_eth_more_t_qr_link1", "https://keyst.one/t/3rd/faq"},
    {"receive_eth_more_t_qr_title1", "Exploring Differences: Standard, Ledger Live, and Legacy Derivation Paths in Ethereum"},
    {"receive_eth_more_t_title1", "Exploring Differences: Standard, Ledger Live, and Legacy Derivation Paths in Ethereum"},
    {"repeat_passcode_desc", "Repeat and confirm the PIN code you’ve just typed."},
    {"repeat_passcode_title", "Repeat the PIN Code"},
    {"seed_check_mid_btn", "Seed Phrase Check"},
    {"seed_check_share_phrase", "Secret Sharing Backup"},
    {"seed_check_share_phrase_title", "Enter your phrase to verify your current wallet seed phrase."},
    {"seed_check_single_phrase", "Single Secret Phrase"},
    {"seed_check_single_phrase_title", "Enter Your Phrase"},
    {"seed_check_verify_match_desc", "Seed phrase matches."},
    {"seed_check_verify_not_match_desc", "Invalid seed phrase. Please try again."},
    {"seed_check_verify_not_match_title", "Verify Failed"},
    {"seed_check_wait_verify", "Verifying"},
    {"set_passcode_desc", "This PIN code will be used to unlock your wallet and sign transactions."},
    {"set_passcode_title", "Set a PIN Code"},
    {"shamir_phrase _desc", "Shamir Backup"},
    {"shamir_phrase_backup_desc", "Write down your share 1 phrase and keep it properly."},
    {"shamir_phrase_backup_title", "Backup Your Phrase"},
    {"shamir_phrase_cancel_create_btn", "Cancel Create"},
    {"shamir_phrase_cancel_create_desc", "Are you sure you want to cancel the process? The confirmed shares will be lost after cancelling."},
    {"shamir_phrase_cancel_create_title", "Cancel Create Wallet?"},
    {"shamir_phrase_confirm_desc", "Select words below in the order of your share phrase to confirm that you have kept it properly."},
    {"shamir_phrase_confirm_title", "Confirm Your Phrase"},
    {"shamir_phrase_continue_btn", "Continue"},
    {"shamir_phrase_custodian_desc", "Please confirm you are the custodian of the share"},
    {"shamir_phrase_custodian_title", "Share"},
    {"shamir_phrase_not_match_desc", "The phrase order is incorrect. Please check your backup and try again"},
    {"shamir_phrase_not_match_title", "Phrase does not match"},
    {"shamir_phrase_notice_desc1", "Anyone with your seed phrase has full access to your cryptocurrency. "},
    {"shamir_phrase_notice_desc2", "Please make sure there are no onlookers or cameras when you record your seed phrase."},
    {"shamir_phrase_notice_title", "Check Your Surroundings"},
    {"shamir_phrase_number", "Number of shares"},
    {"shamir_phrase_threold", "Threshold"},
    {"shamir_phrase_verify_success_desc1", "The seed phrase of this share was verified successfully, now proceed to the next share. "},
    {"shamir_phrase_verify_success_desc2", "Touch the button below and hand over to the second share custodian who keeps share number 2."},
    {"shamir_phrase_verify_success_title", "Verify Successful"},
    {"single_backup_choose_backup_desc", "Choose the method you’d like to backup your seed phrase"},
    {"single_backup_choose_backup_title", "Choose Backup Method"},
    {"single_backup_learn_more_desc", "Shamir backup is a high-security method to  seed phrase. It splits the seed phrase into multiple pieces and determines how many pieces are needed to restore the phrase. You can control how many individuals need to be involved in order to restore the backup to the entire seed phrase."},
    {"single_backup_learn_more_link", "Learn More"},
    {"single_backup_learn_more_qr_link", "https://keyst.one/b/3rd/shamir"},
    {"single_backup_learn_more_qr_title", "What is Shamir Backup?"},
    {"single_backup_learn_more_title", "What is Shamir Backup?"},
    {"single_backup_namewallet_desc", "Give your wallet a name and choose an icon to make your wallet discernible."},
    {"single_backup_namewallet_previntput", "Wallet Name"},
    {"single_backup_namewallet_title", "Name Your Wallet"},
    {"single_backup_notice_desc1", "Anyone with your seed phrase has full access to your cryptocurrency. "},
    {"single_backup_notice_desc2", "Please make sure there are no onlookers or cameras when you record your seed phrase."},
    {"single_backup_notice_title", "Check Your Surroundings"},
    {"single_backup_phrase_regenerate", "Regenerate"},
    {"single_backup_repeatpass_desc", "Repeat and confirm the Password you ve just typed."},
    {"single_backup_repeatpass_title", "Repeat the Password"},
    {"single_backup_repeatpin_desc", "Repeat and confirm the PIN code you ve just typed."},
    {"single_backup_repeatpin_error", "PIN code does not match"},
    {"single_backup_repeatpin_title", "Repeat the PIN Code"},
    {"single_backup_setpass_desc", "This Password will be used to unlock your wallet and sign transactions."},
    {"single_backup_setpass_title", "Set a Password"},
    {"single_backup_setpin_desc", "This PIN code will be used to unlock your wallet and sign transactions."},
    {"single_backup_setpin_title", "Set a PIN Code"},
    {"single_backup_setpin_use_pass", "Use Password"},
    {"single_backup_setpin_use_pin", "Use PIN Code"},
    {"single_backup_shamir_desc", "Backup your seed phrase with higher security level."},
    {"single_backup_shamir_title", "Shamir Backup"},
    {"single_backup_single_phrase_desc", "With a 12 or 24 seed phrase backup. Most commonly used method"},
    {"single_backup_single_phrase_title", "Standard Seed Phrase"},
    {"single_phrase_12words", "12 Words"},
    {"single_phrase_24words", "24 Words"},
    {"single_phrase_confirm_desc", "Tap words below in the order of your Seed phrase to confirm that you have kept it properly."},
    {"single_phrase_confirm_title", "Confirm Your Phrase"},
    {"single_phrase_desc", "Write down your seed phrase in the card and keep it properly"},
    {"single_phrase_low_battery_desc", "The device requires at least 20% power to continue the process"},
    {"single_phrase_low_battery_tilte", "Low Battery"},
    {"single_phrase_not_match_desc", "The phrase order is incorrect. Please check your backup and try again"},
    {"single_phrase_not_match_title", "Phrase does not match"},
    {"single_phrase_reset", "Reset"},
    {"single_phrase_title", "Backup Your Phrase"},
    {"single_phrase_word_amount_select", "Phrase Words Amount"},
    {"tx_details_btc_change_desc", "BTC transactions, based on the UTXO mechanism, allocate some assets to a change address during transfers. This address is generated by the software wallet for anonymity and privacy purposes. You don't need to worry since the change address belongs to your own account, and its amount will be used automatically in subsequent transactions."},
    {"tx_details_btc_change_link", "Learn More"},
    {"tx_details_btc_change_title", "Change Address"},
    {"tx_details_eth_decoding_link", "learn More"},
    {"tx_details_eth_decoding_qr_link", "https://keyst.one/t/3rd/ddt"},
    {"tx_details_eth_decoding_qr_title", "Decoding DeFi Transactions"},
    {"verification_code_desc", "Input this verification code on the Web Authentication page of Keystone's official website to authenticate the device."},
    {"verification_code_failed_desc", "Your device has potentially been breached, putting your sensitive data and digital assets at risk. For safety, erase all personal data and contact our Customer Service team immediately."},
    {"verification_code_failed_link", "support@keyst.one"},
    {"verification_code_failed_title", "Unauthorized breach attempt detected!"},
    {"verification_code_title", "Verification Code"},
    {"verify_cont1", "Please access the link stated below. Click the #F5870A Start Verification# button to initiate the process."},
    {"verify_cont1_link", "https://keyst.one/verify"},
    {"verify_cont2", "Scan the QR code generated on the website to obtain your device verification code."},
    {"verify_cont3", "Enter the code on the website to check whether the device has been compromised or not."},
    {"verify_desc", "This procedure is to verify that the Keystone device or firmware has not been tampered with."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_qr_title", "Verify Your Device"},
    {"verify_scan_invalid_a_desc", "Unable to recognize QR code information"},
    {"verify_scan_invalid_a_title", "Invalid QR Code"},
    {"verify_title", "Verify Your Device"},
    {"wallet_setting_add_wallet", "Add Wallet"},
    {"wallet_setting_passcode", "Fingerprint & Passcode"},
    {"wallet_setting_passphrase", "Passphrase"},
    {"wallet_setting_seed_phrase", "Seed Phrase Check"},
    {"wallet_settings_mid_btn", "Wallet Settings"},
    {"welcome_brand", "Keystone"},
    {"welcome_version", "v0.9.0"},
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


static const char * __lv_i18n_get_text_core(lv_i18n_phrase_t * trans, const char * msg_id)
{
    uint16_t i;
    for (i = 0; trans[i].msg_id != NULL; i++) {
        if (strcmp(trans[i].msg_id, msg_id) == 0) {
            /*The msg_id has found. Check the translation*/
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

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

const static lv_i18n_phrase_t en_singulars[] = {
    {"Address", "Address"},
    {"Primary_Address", "Primary Address"},
    {"Quit", "Quit"},
    {"Sub_Address", "Subaddress"},
    {"Undo", "Undo"},
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
    {"about_info_verify_firmware_desc", " This is an advanced feature for developers to verify that the firmware running on your Forgebox device is consistent with the one we open-sourced."},
    {"about_info_verify_firmware_step1", "Go to Forgebox's open-sourced GitHub repo and follow the instructions to build your firmware and get the checksum."},
    {"about_info_verify_firmware_step2", "Click the #F5870A Checksum# button next to the firmware download."},
    {"about_info_verify_firmware_step3", "Tap the #F5870A Show Checksum# button below and compare the info shown on the webpage and your device."},
    {"about_info_verify_source_code_title", "Verify Source Code"},
    {"about_keystone_discord", "Discord"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "Telegram"},
    {"about_keystone_telegram_url", "t.me/keystonewallet"},
    {"about_keystone_title", "About Forgebox"},
    {"about_keystone_twitter", "Twitter"},
    {"about_keystone_twitter_url", "twitter.com/ForgeboxWallet"},
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
    {"about_terms_product_and_services", "Forgebox Product & Services"},
    {"about_terms_product_and_services_desc", "Our hardware wallet securely manages cryptocurrencies."},
    {"about_terms_prohibited_conduct", "Prohibited Conduct"},
    {"about_terms_prohibited_product_desc", "Our Products and Services are protected by intellectual property laws."},
    {"about_terms_risks", "Risks"},
    {"about_terms_risks_desc", "Be aware of the risks associated with cryptocurrencies and technology vulnerabilities."},
    {"about_terms_subtitle", "Forgebox Terms of Use"},
    {"about_terms_title", "Terms of Use"},
    {"about_terms_website_url", "https://keyst.one/terms"},
    {"calculat_modal_title", "Calculating"},
    {"device_info_title", "Device Info"},
    {"firmware_update_deny_desc", "You need to unlock your device to upgrade the firmware version."},
    {"firmware_update_deny_input_password", "Enter Password"},
    {"firmware_update_deny_title", "Unlock Device Required"},
    {"firmware_update_desc", "To unlock the newest features, please update your firmware to the latest version."},
    {"firmware_update_desc1", "Ensure that your Forgebox has at least 40% battery life remaining."},
    {"firmware_update_desc2", "Go to Forgebox's firmware update page using your computer or smartphone."},
    {"firmware_update_no_upgradable_firmware_desc", "Your device firmware version is higher than or equal to the one on your microSD card."},
    {"firmware_update_no_upgradable_firmware_title", "No Upgradable Firmware Detected"},
    {"firmware_update_sd_checksum_desc", "#F5870A Show Checksum#"},
    {"firmware_update_sd_checksum_done", "Checksum:\r\n"},
    {"firmware_update_sd_checksum_fmt", "#F5870A Show Checksum(%d%%)#"},
    {"firmware_update_sd_checksum_notice", "This is an optional feature to further enhance security. Compare the following checksum with the checksum of your download package on the official website, ensure that they are consistent."},
    {"firmware_update_sd_copying_desc", "Do not remove the MicroSD Card while the update is underway."},
    {"firmware_update_sd_copying_title", "Starting Update"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Transfer the firmware file (forgebox.bin) to your Forgebox using a MicroSD card formatted in FAT32."},
    {"firmware_update_sd_desc4", "Tap the #F5870A Update# button below to initiate the process."},
    {"firmware_update_sd_dialog_desc", "Do you want to update your device's firmware to the new version?"},
    {"firmware_update_sd_dialog_title", "New Firmware Detected"},
    {"firmware_update_sd_failed_access_desc", "Please ensure that you have properly inserted the MicroSD Card."},
    {"firmware_update_sd_failed_access_title", "MicroSD Card Not Detected"},
    {"firmware_update_sd_title", "MicroSD Update"},
    {"firmware_update_title", "Firmware Update"},
    {"firmware_update_updating_desc", "Takes around 10 mins"},
    {"firmware_update_usb_connect_info_desc", "Once connected, the external device will gain permission to transfer data to your Forgebox."},
    {"firmware_update_usb_connect_info_title", "Connect to This Device?"},
    {"firmware_update_usb_desc3", "Connect your Forgebox to your computer using a USB-C cable."},
    {"firmware_update_usb_desc4", "Click the #F5870A Install Update# button on the webpage and follow the instructions to install the latest firmware."},
    {"firmware_update_usb_desc5", "Do not unplug the USB cable while the installation process is underway."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "Firmware Update"},
    {"firmware_update_usb_title", "USB Update"},
    {"firmware_update_usb_title2", "Caution"},
    {"firmware_update_usb_updating_hint", "Do not disconnect the USB cable during the installation process."},
    {"firmware_update_verify_firmware_qr_link", "ForgeboxHQ/forgebox-firmware/docs/verify.md"},
    {"firmware_update_via_sd", "Via MicroSD Card"},
    {"firmware_update_via_usb", "Via USB"},
    {"illustrate_supplement", "."},
    {"language_desc", "Select your language"},
    {"language_little_title", "Language"},
    {"language_option", "English"},
    {"language_title", "Language"},
    {"password_error_not_match", "Passcode mismatch"},
    {"password_error_too_short", "Password must be at least 6 characters"},
    {"sdcard_format_confirm", "Format"},
    {"sdcard_format_desc", "Your MicroSD Card will be formatted to FAT32, erasing all files. Please back up essential files before formatting."},
    {"sdcard_format_failed_desc", "Formatting failed, replace the MicroSD Card or format it on the computer."},
    {"sdcard_format_failed_title", "Formatting Failure"},
    {"sdcard_format_subtitle", "Format MicroSD Card"},
    {"sdcard_format_success_desc", "The MicroSD Card has been successfully formatted to FAT32."},
    {"sdcard_format_success_title", "Formatting Complete"},
    {"sdcard_format_text", "Format MicroSD Card"},
    {"sdcard_formating", "Formatting"},
    {"sdcard_formating_desc", "Do not remove the MicroSD card while formatting is in progress."},
    {"update_success", "Update Successful"},
    {"usb_connection_desc", "When enabled, the USB port can only be used for battery charging purposes."},
    {"usb_connection_subtitle", "Air-Gap Mode"},
    {"usb_connection_title", "Connection"},
    {"verification_code_desc", "Enter this code on Forgebox's official website to verify your device's security."},
    {"verification_code_failed_desc", "Your device may have been compromised, posing a risk to your sensitive data and digital assets.For your safety, we recommend erasing all personal data and contacting Forgebox Support team immediately for assistance."},
    {"verification_code_failed_title", "Unauthorized breach attempt detected!"},
    {"verification_code_title", "Verification Code"},
    {"verification_success", "Verified"},
    {"verify_cont1", "Visit our website and click #F5870A Start Verification# button."},
    {"verify_cont2", "Scan the QR code displayed on the website to get your device verification code."},
    {"verify_cont3", "Enter the code on the website to check for any potential tampering with your device."},
    {"verify_desc", "This process ensures the integrity of your Forgebox device and firmware."},
    {"verify_firmware", "Verify Firmware"},
    {"verify_modal_desc", "Calculating Auth Code..."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "Scan QR Code"},
    {"verify_title", "Verify Your Device"},
    {"verify_title_text", "Verify Your Device"},
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

const static lv_i18n_phrase_t de_singulars[] = {
    {"Address", "Adresse"},
    {"Primary_Address", "Hauptadresse"},
    {"Quit", "Aufhören"},
    {"Sub_Address", "Nebenadresse"},
    {"Undo", "Rückgängig machen"},
    {"about_info_battery_voltage", "Batteriespannung"},
    {"about_info_device_uid", "Gerät UID"},
    {"about_info_export_file_name", "Dateiname:"},
    {"about_info_export_log", "Systemprotokoll exportieren"},
    {"about_info_export_to_sdcard", "Exportieren Sie das Protokoll auf eine MicroSD-Karte."},
    {"about_info_fingerprint_firmware_version", "Fingerabdruck-Firmware-Version"},
    {"about_info_firmware_version", "Firmware-Version"},
    {"about_info_firmware_version_head", "Firmware"},
    {"about_info_result_export_failed", "Export fehlgeschlagen"},
    {"about_info_result_export_failed_desc_no_sdcard", "Bitte stellen Sie sicher, dass Sie eine MicroSD-Karte im Format FAT32 eingefügt haben."},
    {"about_info_result_export_failed_desc_no_space", "Bitte stellen Sie sicher, dass Ihre MicroSD-Karte genügend Speicherplatz hat."},
    {"about_info_result_export_successful", "Export erfolgreich"},
    {"about_info_result_export_successful_desc", "Ihr Systemprotokoll wurde erfolgreich auf die MicroSD-Karte exportiert."},
    {"about_info_serial_number", "Seriennummer"},
    {"about_info_verify_checksum_desc", "Bitte überprüfen Sie, ob die obigen Informationen mit der Webseite übereinstimmen. Wenn sie nicht übereinstimmen, bedeutet dies, dass Ihre Firmware manipuliert worden sein könnte. Bitte verwenden Sie sie sofort nicht mehr."},
    {"about_info_verify_checksum_text", "Prüfsumme"},
    {"about_info_verify_checksum_title", "Prüfsumme"},
    {"about_info_verify_firmware_desc", "Dies ist ein fortgeschrittenes Feature für Entwickler, um zu überprüfen, ob die Firmware auf Ihrem Forgebox-Gerät mit der von uns freigegebenen Firmware übereinstimmt."},
    {"about_info_verify_firmware_step1", "Gehen Sie zum Open-Source-GitHub-Repo von Forgebox und befolgen Sie die Anweisungen, um Ihre Firmware zu erstellen und die Prüfsumme zu erhalten."},
    {"about_info_verify_firmware_step2", "Klicken Sie auf die Schaltfläche #F5870A Prüfsumme# neben dem Firmware-Download."},
    {"about_info_verify_firmware_step3", "Tippen Sie auf die Schaltfläche #F5870A Prüfsumme anzeigen# unten und vergleichen Sie die Informationen, die auf der Webseite und Ihrem Gerät angezeigt werden."},
    {"about_info_verify_source_code_title", "Überprüfe den Quellcode"},
    {"about_keystone_discord", "Discord"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "Telegram"},
    {"about_keystone_telegram_url", "t.me/keystonewallet"},
    {"about_keystone_title", "Über Forgebox"},
    {"about_keystone_twitter", "Twitter"},
    {"about_keystone_twitter_url", "twitter.com/ForgeboxWallet"},
    {"about_keystone_website", "Webseite"},
    {"about_keystone_website_url", "keyst.one"},
    {"about_terms_contact_us", "Kontaktiere uns"},
    {"about_terms_contact_us_desc", "Wenn Sie Fragen oder Bedenken haben, senden Sie uns bitte eine E-Mail an support@keyst.one."},
    {"about_terms_desc", "Um auf die vollständige Version der Nutzungsbedingungen zuzugreifen, besuchen Sie bitte den folgenden Link:"},
    {"about_terms_disclaimers", "Haftungsausschluss"},
    {"about_terms_disclaimers_desc", "Die bereitgestellten Informationen stellen keine Finanzberatung dar. Suchen Sie vor der Entscheidung professionellen Rat."},
    {"about_terms_discontinuance_service", "Einstellung des Dienstes"},
    {"about_terms_discontinuance_service_desc", "Wir können unsere Dienste ändern oder einstellen. Vergessen Sie nicht, Ihren Wiederherstellungsschlüssel zu sichern, um auf Ihre Kryptowährungen zugreifen zu können."},
    {"about_terms_eligibility", "Berechtigung"},
    {"about_terms_eligibility_desc", "Sie müssen mindestens 18 Jahre alt sein, um auf unsere Produkte oder Dienstleistungen zugreifen und sie nutzen zu können."},
    {"about_terms_indemnity", "Entschädigung or Schadenersatz."},
    {"about_terms_law", "Geltendes Recht und Streitbeilegung"},
    {"about_terms_law_desc", "Die Bedingungen unterliegen den Gesetzen der Sonderverwaltungsregion Hongkong, und jeder Streit muss innerhalb eines Jahres eingereicht werden."},
    {"about_terms_modification", "Änderungen dieser Bedingungen"},
    {"about_terms_modification_desc", "Wir behalten uns das Recht vor, diese Bedingungen nach eigenem Ermessen zu ändern."},
    {"about_terms_no_sensitive_information", "Kein Abrufen sensibler Informationen"},
    {"about_terms_no_sensitive_information_desc", "Wir speichern keine sensiblen Informationen wie Passwörter oder Seed-Phrasen. Bewahren Sie Ihre Zugangsdaten sicher auf."},
    {"about_terms_ownership", "Eigentum und Eigentumsrechte"},
    {"about_terms_ownership_desc", "Sie sind für Ihre Handlungen verantwortlich, während Sie die Produkte und Dienstleistungen nutzen."},
    {"about_terms_product_and_services", "Forgebox Produkt- und Dienstleistungen"},
    {"about_terms_product_and_services_desc", "Unsere Hardware-Brieftasche verwaltet Kryptowährungen sicher."},
    {"about_terms_prohibited_conduct", "Verbotenes Verhalten"},
    {"about_terms_prohibited_product_desc", "Unsere Produkte und Dienstleistungen sind durch geistige Eigentumsrechte geschützt."},
    {"about_terms_risks", "Risiken"},
    {"about_terms_risks_desc", "Seien Sie sich der mit Kryptowährungen verbundenen Risiken und Technologieanfälligkeiten bewusst."},
    {"about_terms_subtitle", "Nutzungsbedingungen für Forgebox"},
    {"about_terms_title", "Nutzungsbedingungen"},
    {"about_terms_website_url", "https://keyst.one/terms "},
    {"calculat_modal_title", "Berechnung"},
    {"device_info_title", "Geräteinfo"},
    {"firmware_update_deny_desc", "Sie müssen Ihr Gerät entsperren, um die Firmware-Version zu aktualisieren."},
    {"firmware_update_deny_input_password", "Geben Sie das Passwort ein"},
    {"firmware_update_deny_title", "Gerät entsperren erforderlich"},
    {"firmware_update_desc", "Um die neuesten Funktionen freizuschalten, aktualisieren Sie bitte Ihre Firmware auf die neueste Version"},
    {"firmware_update_desc1", "Stellen Sie sicher, dass Ihr Forgebox noch mindestens 20 % Batterielebensdauer hat."},
    {"firmware_update_desc2", "Gehen Sie mit Ihrem Computer oder Smartphone zur Firmware-Aktualisierungsseite von Forgebox."},
    {"firmware_update_no_upgradable_firmware_desc", "Ihre Geräte-Firmware-Version ist höher als oder gleich der auf Ihrer microSD-Karte."},
    {"firmware_update_no_upgradable_firmware_title", "Keine aktualisierbare Firmware erkannt"},
    {"firmware_update_sd_checksum_desc", "#F5870A Zeige Prüfsumme#"},
    {"firmware_update_sd_checksum_done", "Prüfsumme\r\n"},
    {"firmware_update_sd_checksum_fmt", "#F5870A Prüfsumme anzeigen(%d%%)#"},
    {"firmware_update_sd_checksum_notice", "Dies ist eine optionale Funktion, um die Sicherheit weiter zu verbessern. Vergleichen Sie die folgende Prüfsumme mit der Prüfsumme Ihres Download-Pakets auf der offiziellen Website, stellen Sie sicher, dass sie übereinstimmen."},
    {"firmware_update_sd_copying_desc", "Entfernen Sie die MicroSD-Karte nicht, während das Update im Gange ist."},
    {"firmware_update_sd_copying_title", "Update wird gestartet"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Übertragen Sie die Firmware-Datei (forgebox.bin) mithilfe einer mit FAT32 formatierten MicroSD-Karte auf Ihren Forgebox."},
    {"firmware_update_sd_desc4", "Tippen Sie unten auf die Schaltfläche \"Update\" in der Farbe #F5870A, um den Vorgang zu starten."},
    {"firmware_update_sd_dialog_desc", "Möchten Sie die Firmware Ihres Geräts auf die neue Version aktualisieren?"},
    {"firmware_update_sd_dialog_title", "Neue Firmware erkannt"},
    {"firmware_update_sd_failed_access_desc", "Bitte stellen Sie sicher, dass Sie die MicroSD-Karte ordnungsgemäß eingefügt haben."},
    {"firmware_update_sd_failed_access_title", "Die MicroSD-Karte wird nicht erkannt."},
    {"firmware_update_sd_title", "Die Aktualisierung der MicroSD"},
    {"firmware_update_title", "Firmware-Update"},
    {"firmware_update_updating_desc", "Dauert etwa 10 Minuten."},
    {"firmware_update_usb_connect_info_desc", "Sobald verbunden, erhält das externe Gerät die Erlaubnis, Daten auf Ihren Forgebox zu übertragen."},
    {"firmware_update_usb_connect_info_title", "Mit diesem Gerät verbinden?"},
    {"firmware_update_usb_desc3", "Verbinden Sie Ihren Forgebox mit Ihrem Computer über ein USB-C-Kabel."},
    {"firmware_update_usb_desc4", "Klicken Sie auf die Schaltfläche \"Aktualisieren installieren\" #F5870A# auf der Webseite und folgen Sie den Anweisungen, um die neueste Firmware zu installieren."},
    {"firmware_update_usb_desc5", "Ziehen Sie das USB-Kabel während des Installationsvorgangs nicht ab."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "Firmware-Update"},
    {"firmware_update_usb_title", "USB-Update"},
    {"firmware_update_usb_title2", "Vorsicht"},
    {"firmware_update_usb_updating_hint", "Trennen Sie während des Installationsvorgangs nicht das USB-Kabel."},
    {"firmware_update_verify_firmware_qr_link", "ForgeboxHQ/forgebox-firmware/docs/verify.md"},
    {"firmware_update_via_sd", "Über MicroSD-Karte"},
    {"firmware_update_via_usb", "Via USB"},
    {"illustrate_supplement", "."},
    {"language_desc", "Wählen Sie Ihre Sprache"},
    {"language_little_title", "Sprache"},
    {"language_option", "Deutsch"},
    {"language_title", "Sprache"},
    {"password_error_not_match", "Passcode falsch"},
    {"password_error_too_short", "Das Passwort muss mindestens 6 Zeichen lang sein."},
    {"sdcard_format_confirm", "Format"},
    {"sdcard_format_desc", "Ihre MicroSD-Karte wird zu FAT32 formatiert, wodurch alle Dateien gelöscht werden. Bitte erstellen Sie Sicherungskopien wichtiger Dateien, bevor Sie formatieren."},
    {"sdcard_format_failed_desc", "Das Formatieren ist fehlgeschlagen. Ersetzen Sie die MicroSD-Karte oder formatieren Sie sie am Computer."},
    {"sdcard_format_failed_title", "Formatierungsfehler"},
    {"sdcard_format_subtitle", "MicroSD-Karte formatieren"},
    {"sdcard_format_success_desc", "Die MicroSD-Karte wurde erfolgreich in FAT32 formatiert."},
    {"sdcard_format_success_title", "Formatierung abgeschlossen"},
    {"sdcard_format_text", "MicroSD-Karte formatieren"},
    {"sdcard_formating", "Formatierung"},
    {"sdcard_formating_desc", "Entfernen Sie die MicroSD-Karte nicht, während der Formatierungsvorgang läuft."},
    {"update_success", "Update erfolgreich"},
    {"usb_connection_desc", "Wenn aktiviert, kann der USB-Anschluss nur zum Aufladen des Akkus verwendet werden."},
    {"usb_connection_subtitle", "Air-Gap-Modus"},
    {"usb_connection_title", "Verbindung"},
    {"verification_code_desc", "Geben Sie diesen Code auf der offiziellen Website von Forgebox ein, um die Sicherheit Ihres Geräts zu überprüfen."},
    {"verification_code_failed_desc", "Ihr Gerät könnte kompromittiert worden sein, was ein Risiko für Ihre sensiblen Daten und digitalen Vermögenswerte darstellt. Zu Ihrer Sicherheit empfehlen wir, alle persönlichen Daten zu löschen und sofort das Forgebox-Supportteam zu kontaktieren, um Hilfe zu erhalten"},
    {"verification_code_failed_title", "Unberechtigter Eindringversuch erkannt!"},
    {"verification_code_title", "Bestätigungscode"},
    {"verification_success", "Verifiziert"},
    {"verify_cont1", "Besuchen Sie unsere Website und klicken Sie auf die Schaltfläche #F5870A Start Verification#"},
    {"verify_cont2", "Scannen Sie den auf der Website angezeigten QR-Code, um Ihren Geräteverifizierungscode zu erhalten."},
    {"verify_cont3", "Geben Sie den Code auf der Website ein, um mögliche Manipulationen an Ihrem Gerät zu überprüfen."},
    {"verify_desc", "Dieser Prozess gewährleistet die Integrität Ihres Forgebox-Geräts und der Firmware."},
    {"verify_firmware", "Firmware überprüfen"},
    {"verify_modal_desc", "Berechne Authentifizierungscode..."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "QR-Code scannen"},
    {"verify_title", "Überprüfen Sie Ihr Gerät"},
    {"verify_title_text", "Überprüfen Sie Ihr Gerät"},
    {NULL, NULL} // End mark
};



static uint8_t de_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);
    uint32_t i = op_i(n); UNUSED(i);
    uint32_t v = op_v(n); UNUSED(v);

    if ((i == 1 && v == 0)) return LV_I18N_PLURAL_TYPE_ONE;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t de_lang = {
    .locale_name = "de",
    .singulars = de_singulars,

    .locale_plural_fn = de_plural_fn
};

const static lv_i18n_phrase_t es_singulars[] = {
    {"Address", "Dirección"},
    {"Primary_Address", "Dirección Principal"},
    {"Quit", "Salir"},
    {"Sub_Address", "Dirección Secundaria"},
    {"Undo", "Deshacer"},
    {"about_info_battery_voltage", "Voltaje de la batería"},
    {"about_info_device_uid", "UID del dispositivo"},
    {"about_info_export_file_name", "Nombre de archivo:"},
    {"about_info_export_log", "Exportar el registro del sistema"},
    {"about_info_export_to_sdcard", "Exportar registro a tarjeta MicroSD"},
    {"about_info_fingerprint_firmware_version", "Versión del firmware de huella dactilar"},
    {"about_info_firmware_version", "Versión de firmware"},
    {"about_info_firmware_version_head", "Firmware"},
    {"about_info_result_export_failed", "Exportación fallida"},
    {"about_info_result_export_failed_desc_no_sdcard", "Por favor asegúrate de haber insertado una tarjeta MicroSD formateada en FAT32"},
    {"about_info_result_export_failed_desc_no_space", "Por favor, asegúrate de que tu tarjeta MicroSD tenga suficiente memoria."},
    {"about_info_result_export_successful", "Exportación exitosa"},
    {"about_info_result_export_successful_desc", "El registro de tu sistema ha sido exportado correctamente a la tarjeta MicroSD"},
    {"about_info_serial_number", "Número de serie"},
    {"about_info_verify_checksum_desc", "Por favor verifica si la información anterior coincide con la página web. Si no coincide, significa que tu firmware podría haber sido manipulado. Por favor, deja de usarlo inmediatamente."},
    {"about_info_verify_checksum_text", "Suma de comprobación"},
    {"about_info_verify_checksum_title", "Suma de verificación"},
    {"about_info_verify_firmware_desc", "Esta es una función avanzada para los desarrolladores, que les permite verificar que el firmware que se está ejecutando en tu Forgebox es consistente con el que hemos abierto."},
    {"about_info_verify_firmware_step1", "Ve al repositorio de GitHub de Forgebox de código abierto y sigue las instrucciones para construir tu firmware y obtener el checksum"},
    {"about_info_verify_firmware_step2", "Haz clic en el botón #F5870A Checksum# junto a la descarga del firmware"},
    {"about_info_verify_firmware_step3", "Toca el botón de #F5870A Ver suma de comprobación# a continuación y compara la información mostrada en la página web y en tu dispositivo"},
    {"about_info_verify_source_code_title", "Verificar Código Fuente"},
    {"about_keystone_discord", "Discord"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "Telegram"},
    {"about_keystone_telegram_url", "t.me/keystonewallet"},
    {"about_keystone_title", "Sobre Forgebox"},
    {"about_keystone_twitter", "Twitter"},
    {"about_keystone_twitter_url", "twitter.com/ForgeboxWallet"},
    {"about_keystone_website", "Sitio web"},
    {"about_keystone_website_url", "keyst.one"},
    {"about_terms_contact_us", "Contáctenos"},
    {"about_terms_contact_us_desc", "Si tienes alguna pregunta o inquietud, por favor envíanos un correo electrónico a support@keyst.one."},
    {"about_terms_desc", "Para acceder a la versión completa de los TÉRMINOS DE USO, visita el siguiente enlace:"},
    {"about_terms_disclaimers", "DESCARGO DE RESPONSABILIDAD"},
    {"about_terms_disclaimers_desc", "La información proporcionada no constituye asesoramiento financiero. Busque el asesoramiento de un profesional antes de tomar cualquier decisión"},
    {"about_terms_discontinuance_service", "Interrupción del servicio"},
    {"about_terms_discontinuance_service_desc", "Podemos modificar o interrumpir nuestros servicios. Recuerda hacer una copia de seguridad de tu frase semilla para acceder a tus criptomonedas"},
    {"about_terms_eligibility", "Elegibilidad"},
    {"about_terms_eligibility_desc", "Debes tener 18 años o más para acceder y usar nuestros Productos o Servicios"},
    {"about_terms_indemnity", "Indemnización"},
    {"about_terms_law", "Ley aplicable y resolución de disputas"},
    {"about_terms_law_desc", "Los Términos están regidos por las leyes de la RAS de Hong Kong y cualquier disputa debe presentarse dentro de un año"},
    {"about_terms_modification", "Modificación de estos Términos"},
    {"about_terms_modification_desc", "Nos reservamos el derecho de cambiar estos Términos a nuestra discreción"},
    {"about_terms_no_sensitive_information", "No hay recuperación de información confidencial"},
    {"about_terms_no_sensitive_information_desc", "No almacenamos información delicada como contraseñas o frases de recuperación. Mantén tus credenciales seguras"},
    {"about_terms_ownership", "Propiedad y derechos de propiedad"},
    {"about_terms_ownership_desc", "Eres responsable de tus acciones al usar los Productos y Servicios"},
    {"about_terms_product_and_services", "Productos y Servicios Forgebox"},
    {"about_terms_product_and_services_desc", "Nuestra billetera de hardware administra de forma segura criptomonedas"},
    {"about_terms_prohibited_conduct", "Conducta Prohibida"},
    {"about_terms_prohibited_product_desc", "Nuestros productos y servicios están protegidos por leyes de propiedad intelectual"},
    {"about_terms_risks", "Riesgos"},
    {"about_terms_risks_desc", "Ten en cuenta los riesgos asociados con las criptomonedas y las vulnerabilidades tecnológicas"},
    {"about_terms_subtitle", "Términos de Uso de Forgebox"},
    {"about_terms_title", "Términos de Uso"},
    {"about_terms_website_url", "https://keyst.one/terms"},
    {"calculat_modal_title", "Calculando"},
    {"device_info_title", "Información del dispositivo"},
    {"firmware_update_deny_desc", "Necesitas desbloquear tu dispositivo para actualizar la versión del firmware"},
    {"firmware_update_deny_input_password", "Introduce la contraseña"},
    {"firmware_update_deny_title", "Es necesario desbloquear el dispositivo"},
    {"firmware_update_desc", "Para desbloquear las características más recientes, por favor actualiza el firmware a la última versión."},
    {"firmware_update_desc1", "Asegúrate de que tu Forgebox tenga al menos un 40% de batería restante"},
    {"firmware_update_desc2", "Ve a la página de actualización de firmware de Forgebox utilizando tu computadora o teléfono inteligente"},
    {"firmware_update_no_upgradable_firmware_desc", "La versión del firmware de tu dispositivo es superior o igual a la que está en tu tarjeta microSD"},
    {"firmware_update_no_upgradable_firmware_title", "No se detectó firmware actualizable"},
    {"firmware_update_sd_checksum_desc", "#F5870A Mostrar Suma de Verificación#"},
    {"firmware_update_sd_checksum_done", "Suma de verificación:"},
    {"firmware_update_sd_checksum_fmt", "#F5870A Mostrar Suma de Verificación(%d%%)#"},
    {"firmware_update_sd_checksum_notice", "Esta es una característica opcional para aumentar aún más la seguridad. Compara el siguiente checksum con el checksum de tu paquete de descarga en el sitio web oficial, asegúrate de que sean consistentes."},
    {"firmware_update_sd_copying_desc", "No retires la tarjeta MicroSD mientras la actualización esté en curso"},
    {"firmware_update_sd_copying_title", "Actualización en curso"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Transfiere el archivo de firmware (forgebox.bin) a tu Forgebox utilizando una tarjeta MicroSD formateada en FAT32"},
    {"firmware_update_sd_desc4", "Toca el botón 'Actualizar' #F5870A que se encuentra debajo para iniciar el proceso"},
    {"firmware_update_sd_dialog_desc", "Una nueva versión de firmware está disponible. ¿Quieres actualizar el firmware de tu dispositivo a la nueva versión?"},
    {"firmware_update_sd_dialog_title", "Actualización disponible"},
    {"firmware_update_sd_failed_access_desc", "Por favor, asegúrate de haber insertado correctamente la tarjeta MicroSD."},
    {"firmware_update_sd_failed_access_title", "Tarjeta MicroSD no detectada"},
    {"firmware_update_sd_title", "Actualización de MicroSD"},
    {"firmware_update_title", "Actualización de firmware"},
    {"firmware_update_updating_desc", "Toma alrededor de 10 minutos"},
    {"firmware_update_usb_connect_info_desc", "Una vez conectado, el dispositivo externo obtendrá permiso para transferir datos a tu Forgebox."},
    {"firmware_update_usb_connect_info_title", "¿Conectarse a este dispositivo?"},
    {"firmware_update_usb_desc3", "Conecta tu Forgebox a tu computadora usando un cable USB-C"},
    {"firmware_update_usb_desc4", "Haz clic en el botón #F5870A Instalar Actualización# en la página web y sigue las instrucciones para instalar el firmware más reciente"},
    {"firmware_update_usb_desc5", "No desconectes el cable USB mientras el proceso de instalación esté en curso"},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "Actualización del firmware"},
    {"firmware_update_usb_title", "Actualización de USB"},
    {"firmware_update_usb_title2", "Precaución"},
    {"firmware_update_usb_updating_hint", "No desconectes el cable USB durante el proceso de instalación"},
    {"firmware_update_verify_firmware_qr_link", "ForgeboxHQ/forgebox-firmware/docs/verify.md"},
    {"firmware_update_via_sd", "A través de la tarjeta MicroSD"},
    {"firmware_update_via_usb", "A través de USB"},
    {"illustrate_supplement", ""},
    {"language_desc", "Selecciona tu idioma"},
    {"language_little_title", "Idioma"},
    {"language_option", "Español"},
    {"language_title", "Idioma"},
    {"password_error_not_match", "incompatibilidad de código de acceso"},
    {"password_error_too_short", "La contraseña debe tener al menos 6 caracteres"},
    {"sdcard_format_confirm", "Formato"},
    {"sdcard_format_desc", "Tu tarjeta MicroSD se formateará a FAT32, borrando todos los archivos. Por favor, realiza una copia de seguridad de los archivos esenciales antes de formatear."},
    {"sdcard_format_failed_desc", "El formateo falló, reemplaza la tarjeta MicroSD o formatea en la computadora."},
    {"sdcard_format_failed_title", "Fallo de formato"},
    {"sdcard_format_subtitle", "Formatear tarjeta MicroSD"},
    {"sdcard_format_success_desc", "La tarjeta MicroSD ha sido formateada con éxito a FAT32."},
    {"sdcard_format_success_title", "Formateo Completo"},
    {"sdcard_format_text", "Formatear tarjeta MicroSD"},
    {"sdcard_formating", "Formateo"},
    {"sdcard_formating_desc", "No retires la tarjeta MicroSD mientras se está formateando"},
    {"update_success", "Actualización exitosa"},
    {"usb_connection_desc", "Cuando está habilitado, el puerto USB solo puede utilizarse para cargar la batería."},
    {"usb_connection_subtitle", "Modo Air-Gap"},
    {"usb_connection_title", "Conexión"},
    {"verification_code_desc", "Ingresa este código en el sitio web oficial de Forgebox para verificar la seguridad de tu dispositivo"},
    {"verification_code_failed_desc", "Tu dispositivo puede haber sido comprometido, lo que representa un riesgo para tus datos sensibles y activos digitales. \r\nPara tu seguridad, recomendamos borrar todos los datos personales y contactar de inmediato al equipo de soporte de Forgebox para obtener ayuda."},
    {"verification_code_failed_title", "¡Se ha detectado un intento de violación no autorizada!"},
    {"verification_code_title", "Código de verificación"},
    {"verification_success", "Verificado"},
    {"verify_cont1", "Visita nuestro sitio web y haz clic en el botón #F5870A Iniciar verificación"},
    {"verify_cont2", "Escanea el código QR que se muestra en el sitio web para obtener tu código de verificación del dispositivo"},
    {"verify_cont3", "Ingresa el código en el sitio web para verificar cualquier posible manipulación de tu dispositivo"},
    {"verify_desc", "Este proceso asegura la integridad de tu dispositivo Forgebox y el firmware"},
    {"verify_firmware", "Verificar firmware"},
    {"verify_modal_desc", "Calculando código de autorización..."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "Escanear código QR"},
    {"verify_title", "Verifica tu dispositivo"},
    {"verify_title_text", "Verifica tu dispositivo"},
    {NULL, NULL} // End mark
};



static uint8_t es_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);


    if ((n == 1)) return LV_I18N_PLURAL_TYPE_ONE;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t es_lang = {
    .locale_name = "es",
    .singulars = es_singulars,

    .locale_plural_fn = es_plural_fn
};

const static lv_i18n_phrase_t ja_singulars[] = {
    {"Address", "住所"},
    {"Primary_Address", "主要住所"},
    {"Quit", "辞める"},
    {"Sub_Address", "サブ住所"},
    {"Undo", "アンドゥ"},
    {"about_info_battery_voltage", "バッテリー電圧"},
    {"about_info_device_uid", "デバイスUID"},
    {"about_info_export_file_name", "ファイル名:"},
    {"about_info_export_log", "エクスポートシステムログ"},
    {"about_info_export_to_sdcard", "マイクロSDカードにログをエクスポート"},
    {"about_info_fingerprint_firmware_version", "指紋のファームウェアバージョン"},
    {"about_info_firmware_version", "ファームウェアバージョン"},
    {"about_info_firmware_version_head", "ファームウェア"},
    {"about_info_result_export_failed", "エクスポートに失敗しました."},
    {"about_info_result_export_failed_desc_no_sdcard", "FAT32でフォーマットされたMicroSDカードが正しく挿入されていることをご確認ください."},
    {"about_info_result_export_failed_desc_no_space", "MicroSDカードの容量が十分であることを確認してください."},
    {"about_info_result_export_successful", "エクスポートが成功しました."},
    {"about_info_result_export_successful_desc", "システムログはMicroSDカードに正常にエクスポートされました."},
    {"about_info_serial_number", "シリアルナンバー"},
    {"about_info_verify_checksum_desc", "上記の情報がウェブページと一致しているか確認してください.一致しない場合、ファームウェアが改ざんされている可能性があります.即座に使用を中止してください."},
    {"about_info_verify_checksum_text", "チェックサム"},
    {"about_info_verify_checksum_title", "チェックサム"},
    {"about_info_verify_firmware_desc", "これは開発者向けの高度な機能で、 Forgeboxデバイス上で実行されるファームウェアがオープンソース化されたものと一致していることを検証するためのものです"},
    {"about_info_verify_firmware_step1", "これは開発者向けの高度な機能で,Forgebox デバイス上で実行されるファームウェアがオ ープンソース化されたものと一致して いることを検証するためのものです"},
    {"about_info_verify_firmware_step2", "ファームウェアのダウンロード横にある「#F5870A チェックサム#」ボタンをクリックしてください."},
    {"about_info_verify_firmware_step3", "以下のウェブページとデバイス上に表示された情報を比較するために、下の#F5870A サムチェックボタン#をタップしてください."},
    {"about_info_verify_source_code_title", "ソースコードを検証する"},
    {"about_keystone_discord", "ディスコード"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "テレグラム"},
    {"about_keystone_telegram_url", "t.me/keystonewallet"},
    {"about_keystone_title", "Forgeboxについて"},
    {"about_keystone_twitter", "ツイッター"},
    {"about_keystone_twitter_url", "twitter.com/ForgeboxWallet"},
    {"about_keystone_website", "ウェブサイト"},
    {"about_keystone_website_url", "keyst.one"},
    {"about_terms_contact_us", "お問い合わせ"},
    {"about_terms_contact_us_desc", "ご質問やご心配事がございましたら、support@keyst.one までメールでお問い合わせください."},
    {"about_terms_desc", "「ご利用条件」の全文にアクセスするには、以下のリンクをご覧ください."},
    {"about_terms_disclaimers", "免責事項"},
    {"about_terms_disclaimers_desc", "提供された情報は金融アドバイスではありません.どんな決定をする前に、専門家のアドバイスを求めてください."},
    {"about_terms_discontinuance_service", "サービスの終了"},
    {"about_terms_discontinuance_service_desc", "私たちはサービスを修正または中止する可能性があります.暗号通貨にアクセスするためにシードフレーズをバックアップすることを忘れずに."},
    {"about_terms_eligibility", "資格"},
    {"about_terms_eligibility_desc", "私たちの製品またはサービスを利用するには、18歳以上である必要があります."},
    {"about_terms_indemnity", "保証"},
    {"about_terms_law", "「法律の適用および紛争解決」"},
    {"about_terms_law_desc", "条件は香港SAR法に基づきます.紛争は1年以内に申し立てる必要があります."},
    {"about_terms_modification", "「これらの利用規約の変更」"},
    {"about_terms_modification_desc", "私たちは、これらの条件を自己の裁量により変更する権利を留保します."},
    {"about_terms_no_sensitive_information", "機密情報の取得はありません."},
    {"about_terms_no_sensitive_information_desc", "私たちはパスワードやシードフレーズのような重要な情報を保存しません.自分の認証情報を安全に保ってください."},
    {"about_terms_ownership", "所有権と所有権に関する権利"},
    {"about_terms_ownership_desc", "製品およびサービスの利用中は、自分の行動に責任を持つ必要があります."},
    {"about_terms_product_and_services", "キーストーンプロダクト＆サービス"},
    {"about_terms_product_and_services_desc", "私たちのハードウェアウォレットは暗号通貨を安全に管理します."},
    {"about_terms_prohibited_conduct", "禁止行為"},
    {"about_terms_prohibited_product_desc", "私たちの製品やサービスは知的財産法で保護されています."},
    {"about_terms_risks", "リスク"},
    {"about_terms_risks_desc", "仮想通貨や技術の脆弱性に関連するリスクに注意してください."},
    {"about_terms_subtitle", "Forgebox利用規約"},
    {"about_terms_title", "利用規約"},
    {"about_terms_website_url", "https://keyst.one/terms"},
    {"calculat_modal_title", "計算しています"},
    {"device_info_title", "デバイス情報"},
    {"firmware_update_deny_desc", "デバイスのファームウェアバージョンをアップグレードするためには、デバイスのロックを解除する必要があります."},
    {"firmware_update_deny_input_password", "パスワードを入力してください."},
    {"firmware_update_deny_title", "デバイスのロック解除が必要です."},
    {"firmware_update_desc", "最新の機能を利用するためには、ファームウェアを最新バージョンにアップデートしてください"},
    {"firmware_update_desc1", "Forgeboxが少なくとも40%のバッテリー残量を確保してください."},
    {"firmware_update_desc2", "コンピュータまたはスマートフォンを使用して、Forgeboxのファームウェアのアップデートページにアクセスしてください."},
    {"firmware_update_no_upgradable_firmware_desc", "あなたのデバイスのファームウェアバージョンは、マイクロSDカードに保存されているものと同じかそれよりも高いです."},
    {"firmware_update_no_upgradable_firmware_title", "アップグレード可能なファームウェアが検出されませんでした."},
    {"firmware_update_sd_checksum_desc", "#F5870A チェックサムを表示#"},
    {"firmware_update_sd_checksum_done", "チェックサム:\r\n"},
    {"firmware_update_sd_checksum_fmt", "#F5870A チェックサムを表示(%d%%) #"},
    {"firmware_update_sd_checksum_notice", "これはセキュリティをさらに強化するためのオプション機能です.公式ウェブサイトでのダウンロードパッケージのチェックサムと以下のチェックサムを比較し、一致することを確認してください."},
    {"firmware_update_sd_copying_desc", "アップデート中はMicroSDカードを取り外さないでください."},
    {"firmware_update_sd_copying_title", "アップデートを開始します"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Forgeboxにファームウェアファイル(forgebox.bin)をMicroSDカードに転送し、そのカードをFAT32形式でフォーマットしてください."},
    {"firmware_update_sd_desc4", "プロセスを開始するには、下の#F5870Aアップデート#ボタンをタップしてください"},
    {"firmware_update_sd_dialog_desc", "デバイスのファームウェアを新しいバージョンに更新しますか?"},
    {"firmware_update_sd_dialog_title", "新しいファームウェアが検出されました"},
    {"firmware_update_sd_failed_access_desc", "マイクロSDカードが正しく挿入されていることを確認してください."},
    {"firmware_update_sd_failed_access_title", "マイクロSDカードが検出されていません."},
    {"firmware_update_sd_title", "マイクロSDのアップデート"},
    {"firmware_update_title", "ファームウェアアップデート"},
    {"firmware_update_updating_desc", "約10分かかります."},
    {"firmware_update_usb_connect_info_desc", "接続が完了すると、外部デバイスはForgeboxにデータを転送する許可が得られます."},
    {"firmware_update_usb_connect_info_title", "このデバイスに接続しますか?"},
    {"firmware_update_usb_desc3", "「USB-Cケーブルを使用して、キーストーンをコンピュータに接続してください.」"},
    {"firmware_update_usb_desc4", "ウェブページ上の「#F5870A インストールの更新#」ボタンをクリックし、最新のファームウェアをインストールするための指示に従ってください."},
    {"firmware_update_usb_desc5", "インストールプロセスが進行中の間は、USBケーブルを抜かないでください."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "ファームウェアアップデート"},
    {"firmware_update_usb_title", "USBアップデート"},
    {"firmware_update_usb_title2", "注意"},
    {"firmware_update_usb_updating_hint", "インストールプロセス中にUSBケーブルを切断しないでください."},
    {"firmware_update_verify_firmware_qr_link", "ForgeboxHQ/forgebox-firmware/docs/verify.md"},
    {"firmware_update_via_sd", "マイクロSDカード経由"},
    {"firmware_update_via_usb", "USB経由で"},
    {"illustrate_supplement", "秒"},
    {"language_desc", "言語を選択してください."},
    {"language_little_title", "言語"},
    {"language_option", "日本語"},
    {"language_title", "言語"},
    {"password_error_not_match", "パスコードが一致しません"},
    {"password_error_too_short", "パスワードは最低6文字以上必要です."},
    {"sdcard_format_confirm", "フォーマット"},
    {"sdcard_format_desc", "「お使いのMicroSDカードはFAT32形式にフォーマットされ、すべてのファイルが削除されます.フォーマット前に重要なファイルをバックアップしてください.」"},
    {"sdcard_format_failed_desc", "フォーマットに失敗しました.MicroSDカードを交換するか、コンピュータでフォーマットしてください."},
    {"sdcard_format_failed_title", "フォーマットの失敗"},
    {"sdcard_format_subtitle", "マイクロSDカードをフォーマット"},
    {"sdcard_format_success_desc", "マイクロSDカードは正常にFAT32にフォーマットされました."},
    {"sdcard_format_success_title", "フォーマットが完了しました"},
    {"sdcard_format_text", "マイクロSDカードをフォーマット"},
    {"sdcard_formating", "フォーマット"},
    {"sdcard_formating_desc", "フォーマット中はMicroSDカードを取り外さないでください."},
    {"update_success", "更新が完了しました."},
    {"usb_connection_desc", "有効になっている場合、USBポートはバッテリーの充電用にしか使用できません."},
    {"usb_connection_subtitle", "エアギャップモード"},
    {"usb_connection_title", "接続"},
    {"verification_code_desc", "Forgeboxの公式ウェブサイトにこのコードを入力して、デバイスのセキュリティを確認してください."},
    {"verification_code_failed_desc", "お使いのデバイスは侵害された可能性があり、機密データおよびデジタル資産にリスクが及ぶことがあります。安全のため、すべての個人データを消去し、直ちにForgeboxサポートチームに連絡して支援を受けることをお勧めします"},
    {"verification_code_failed_title", "不正な侵入試行が検出されました！"},
    {"verification_code_title", "認証コード"},
    {"verification_success", "検証済み"},
    {"verify_cont1", "ウェブサイトを訪れて、#F5870A 認証を開始#ボタンをクリックしてください"},
    {"verify_cont2", "ウェブサイトに表示されているQRコードをスキャンして、デバイスの認証コードを取得してください."},
    {"verify_cont3", "ウェブサイト上でコードを入力して、デバイスに何らかの改ざんが行われたかどうかを確認してください."},
    {"verify_desc", "このプロセスは、Forgeboxデバイスとファームウェアの信頼性を保証します."},
    {"verify_firmware", "ファームウェアを検証する"},
    {"verify_modal_desc", "認証コードを計算中..."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "QRコードをスキャンしてください"},
    {"verify_title", "デバイスを確認してください."},
    {"verify_title_text", "デバイスを確認してください."},
    {NULL, NULL} // End mark
};



static uint8_t ja_plural_fn(int32_t num)
{



    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t ja_lang = {
    .locale_name = "ja",
    .singulars = ja_singulars,

    .locale_plural_fn = ja_plural_fn
};

const static lv_i18n_phrase_t ko_singulars[] = {
    {"Address", "주소"},
    {"Primary_Address", "기본 주소"},
    {"Quit", "종료"},
    {"Sub_Address", "하위 주소"},
    {"Undo", "되돌리다"},
    {"about_info_battery_voltage", "배터리 전압"},
    {"about_info_device_uid", "장치 UID"},
    {"about_info_export_file_name", "파일명"},
    {"about_info_export_log", "시스템 로그 내보내기"},
    {"about_info_export_to_sdcard", "MicroSD 카드로 로그 내보내기"},
    {"about_info_fingerprint_firmware_version", "지문 펌웨어 버전"},
    {"about_info_firmware_version", "펌웨어 버전"},
    {"about_info_firmware_version_head", "펌웨어"},
    {"about_info_result_export_failed", "내보내기 실패"},
    {"about_info_result_export_failed_desc_no_sdcard", "FAT32로 포맷된 MicroSD 카드를 삽입했는지 확인하십시오."},
    {"about_info_result_export_failed_desc_no_space", "MicroSD 카드에 충분한 메모리가 있는지 확인해 주세요."},
    {"about_info_result_export_successful", "내보내기 성공"},
    {"about_info_result_export_successful_desc", "시스템 로그를 MicroSD 카드로 내보냈습니다."},
    {"about_info_serial_number", "제조 번호"},
    {"about_info_verify_checksum_desc", "만약 위의 정보가 자체적으로 계산한 mh1903.bin 파일의 체크섬 일치하면 오픈 소스 펌웨어가 장치의 펌웨어와 일치함을 의미합니다."},
    {"about_info_verify_checksum_text", "체크섬"},
    {"about_info_verify_checksum_title", "체크섬"},
    {"about_info_verify_firmware_desc", "이 기능은 개발자들이 키스톤 장치에서 실행중인 펌웨어가 우리가 오픈 소스로 공개한 펌웨어와 일치하는지 확인하기 위한 고급 기능입니다"},
    {"about_info_verify_firmware_step1", "Forgebox의 오픈 소스 GitHub 저장소로 이동하여 펌웨어를 빌드하고 체크섬을 얻는 지침을 따르세요"},
    {"about_info_verify_firmware_step2", "펌웨어 페이지에서 \"펌웨어 다운로드\" 버튼 옆의 #F5870A Checksum#을 클릭합니다."},
    {"about_info_verify_firmware_step3", "아래의 #F5870A 체크섬 보기# 버튼을 눌러 웹페이지에 표시된 정보와 장치의 정보를 비교하세요"},
    {"about_info_verify_source_code_title", "소스 코드 검증"},
    {"about_keystone_discord", "Discord"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "Telegram"},
    {"about_keystone_telegram_url", "t.me/keystonewallet"},
    {"about_keystone_title", "About Forgebox"},
    {"about_keystone_twitter", "Twitter"},
    {"about_keystone_twitter_url", "twitter.com/ForgeboxWallet"},
    {"about_keystone_website", "웹사이트"},
    {"about_keystone_website_url", "keyst.one"},
    {"about_terms_contact_us", "우리에게 연락하세요"},
    {"about_terms_contact_us_desc", "질문이나 우려 사항이 있으시면 support@keyst.one으로 이메일을 보내주시기 바랍니다."},
    {"about_terms_desc", "사용 약관의 전체 버전에 액세스하려면 다음 링크를 참조하십시오:"},
    {"about_terms_disclaimers", "면책사항"},
    {"about_terms_disclaimers_desc", "제공된 정보는 재정적인 조언이 아닙니다. 어떤 결정을 내리기 전에 전문적인 조언을 구하세요."},
    {"about_terms_discontinuance_service", "서비스 중단"},
    {"about_terms_discontinuance_service_desc", "서비스를 수정하거나 중단할 수 있습니다. 암호화폐에 액세스하려면 시드 문구를 백업하는 것을 기억하십시오."},
    {"about_terms_eligibility", "자격"},
    {"about_terms_eligibility_desc", "당사의 제품 또는 서비스에 액세스하고 사용하려면 18세 이상이어야 합니다."},
    {"about_terms_indemnity", "보장"},
    {"about_terms_law", "관련법과 분쟁해결"},
    {"about_terms_law_desc", "본 약관은 홍콩 특별행정구법에 따라 적용되며, 모든 분쟁은 1년 이내에 제기되어야 합니다."},
    {"about_terms_modification", "본 약관 수정"},
    {"about_terms_modification_desc", "당사는 본 약관을 당사의 재량에 따라 변경할 수 있는 권리를 보유합니다."},
    {"about_terms_no_sensitive_information", "중요한 정보를 검색하지 않음"},
    {"about_terms_no_sensitive_information_desc", "비밀번호나 시드 문구와 같은 중요한 정보는 저장하지 않습니다. 자격 증명을 안전하게 유지하십시오."},
    {"about_terms_ownership", "소유권 및 독점권"},
    {"about_terms_ownership_desc", "귀하는 제품 및 서비스를 사용하는 동안 귀하의 행동에 책임이 있습니다."},
    {"about_terms_product_and_services", "키스톤 제품 및 서비스"},
    {"about_terms_product_and_services_desc", "당사의 하드웨어 지갑은 암호화폐를 안전하게 관리합니다."},
    {"about_terms_prohibited_conduct", "금지된 행위"},
    {"about_terms_prohibited_product_desc", "당사의 제품 및 서비스는 지적 재산권법에 의해 보호됩니다."},
    {"about_terms_risks", "위험"},
    {"about_terms_risks_desc", "암호화폐 및 기술 취약성과 관련된 위험을 인식해야 합니다."},
    {"about_terms_subtitle", "키스톤 이용약관"},
    {"about_terms_title", "이용약관"},
    {"about_terms_website_url", "https://keyst.one/terms"},
    {"calculat_modal_title", "계산중 "},
    {"device_info_title", "장치 정보"},
    {"firmware_update_deny_desc", "펌웨어 버전을 업그레이드하려면 장치의 잠금을 해제해야 합니다."},
    {"firmware_update_deny_input_password", "비밀번호 입력"},
    {"firmware_update_deny_title", "장치 잠금 해제 필요합니다. "},
    {"firmware_update_desc", "최신 기능을 경험하시려면 최신 펌웨어를 다운로드하십시오!"},
    {"firmware_update_desc1", "장치 배터리가 40% 이상인지 확인하십시오."},
    {"firmware_update_desc2", "컴퓨터 또는 스마트폰을 사용하여 Forgebox의 펌웨어 업데이트 페이지로 이동합니다."},
    {"firmware_update_no_upgradable_firmware_desc", "장치의 현재 버전이 MicroSD 카드의 펌웨어 버전보다 높거나 같습니다."},
    {"firmware_update_no_upgradable_firmware_title", "업그레이드 가능한 펌웨어를 찾을 수 없습니다."},
    {"firmware_update_sd_checksum_desc", "#F5870A 체크섬 표시#"},
    {"firmware_update_sd_checksum_done", "체크섬\r\n"},
    {"firmware_update_sd_checksum_fmt", "#F5870A 체크섬 표시 (%d%%)#"},
    {"firmware_update_sd_checksum_notice", "보안성을 더욱 높일 수 있는 옵션 기능입니다.아래 체크섬과 공식 홈페이지에서 다운로드한 패키지의 체크섬을 비교하여 동일하게 유지하여 주시기 바랍니다."},
    {"firmware_update_sd_copying_desc", "업그레이드 중에는 microSD 카드를 제거하지 마십시오."},
    {"firmware_update_sd_copying_title", "업데이트 시작"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "펌웨어(forgebox.bin)를 microSD(포맷: FAT32)에 다운로드하고 SD 카드를 장치에 연결합니다."},
    {"firmware_update_sd_desc4", "하단의 #F5870A 업데이트 #버튼을 누르시면 업그레이드가 시작됩니다."},
    {"firmware_update_sd_dialog_desc", "새 버전으로 기기의 펌웨어를 업데이트하시겠습니까?"},
    {"firmware_update_sd_dialog_title", "새 펌웨어가 감지되었습니다"},
    {"firmware_update_sd_failed_access_desc", "MicroSD 카드가 올바르게 연결되었는지 확인하십시오."},
    {"firmware_update_sd_failed_access_title", "MicroSD 카드 인식 실패"},
    {"firmware_update_sd_title", "microSD 업데이트"},
    {"firmware_update_title", "펌웨어 업데이트"},
    {"firmware_update_updating_desc", "약 10분 소요"},
    {"firmware_update_usb_connect_info_desc", "연결되면 외부 장치는 데이터를 Forgebox으로 전송할 수 있는 권한을 갖게 됩니다."},
    {"firmware_update_usb_connect_info_title", "이 장치에 연결하시겠습니까?"},
    {"firmware_update_usb_desc3", "USB-C 케이블을 사용하여 장치를 컴퓨터에 연결합니다."},
    {"firmware_update_usb_desc4", "\"WebUSB 펌웨어 업그레이드\" 페이지에서 #F5870A 연결 시작 #버튼을 클릭한 다음 안내를 따라 최신 펌웨어를 설치합니다."},
    {"firmware_update_usb_desc5", "업그레이드 중, USB 연결을 유지해 주세요."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "펌웨어 업데이트"},
    {"firmware_update_usb_title", "USB 업데이트"},
    {"firmware_update_usb_title2", "경고"},
    {"firmware_update_usb_updating_hint", "업그레이드 중, USB 연결을 유지해 주세요."},
    {"firmware_update_verify_firmware_qr_link", "ForgeboxHQ/forgebox-firmware/docs/verify.md "},
    {"firmware_update_via_sd", "MicroSD 카드"},
    {"firmware_update_via_usb", "USB"},
    {"illustrate_supplement", "."},
    {"language_desc", "언어 선택"},
    {"language_little_title", "언어"},
    {"language_option", "한국어"},
    {"language_title", "언어"},
    {"password_error_not_match", "비밀번호 불일치"},
    {"password_error_too_short", "비밀번호는 6자 이상이어야 합니다."},
    {"sdcard_format_confirm", "포맷"},
    {"sdcard_format_desc", "귀하의 MicroSD 카드는 FAT32로 포맷되고 모든 파일이 삭제될 것입니다.포맷하기 전에 중요한 파일을 백업해 주세요."},
    {"sdcard_format_failed_desc", "포맷에 실패했습니다. MicroSD 카드를 교체하거나 컴퓨터에서 포맷하십시오."},
    {"sdcard_format_failed_title", "포맷 실패"},
    {"sdcard_format_subtitle", "마이크로SD 카드 포맷"},
    {"sdcard_format_success_desc", "MicroSD 카드가 FAT32로 성공적으로 포맷되었습니다."},
    {"sdcard_format_success_title", "포맷 완료"},
    {"sdcard_format_text", "마이크로SD 카드 포맷"},
    {"sdcard_formating", "포맷 진행중 "},
    {"sdcard_formating_desc", "포맷 과정에서 MicroSD 카드를 제거하지 마십시오."},
    {"update_success", "업그레이드 성공"},
    {"usb_connection_desc", "활성화된 경우 usb는 배터리 충전에만 사용할 수 있습니다"},
    {"usb_connection_subtitle", "에어갭 모드"},
    {"usb_connection_title", "연결"},
    {"verification_code_desc", "\"장치 확인\" 페이지에 인증 코드를 입력하여 장치 안전성을 확인할 수 있습니다. "},
    {"verification_code_failed_desc", "장치는 잠재적으로 변조될 수 있으며, 개인 데이터 및 디지털 자산에 위협이 될 수 있습니다.자산 보안을 위해 모든 개인 데이터를 삭제하고 즉시 키스톤 지원팀에 연락하여 도움을 요청할 것을 권장합니다."},
    {"verification_code_failed_title", "무단 위반 시도가 감지되었습니다!"},
    {"verification_code_title", "인증 코드"},
    {"verification_success", "인증 성공"},
    {"verify_cont1", "저희 웹사이트에 방문하셔서 #F5870A 장치확인 #버튼을 클릭해주세요."},
    {"verify_cont2", "\"장치 확인\" 페이지에 표시된 QR 코드를 스캔하면 장치 인증 코드를 받을 수 있습니다."},
    {"verify_cont3", "\"장치 확인\" 페이지에 인증 코드를 입력하여 장치의 변조 여부를 확인합니다. "},
    {"verify_desc", "장치 확인 기능을 통해 장치가 변조되었는지 확인할 수 있으며 펌웨어의 무결성을 확인하는 데 사용할 수도 있습니다."},
    {"verify_firmware", "펌웨어 검증"},
    {"verify_modal_desc", "인증 코드 받기 "},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "QR코드 스캔"},
    {"verify_title", "장치 확인"},
    {"verify_title_text", "장치 확인"},
    {NULL, NULL} // End mark
};



static uint8_t ko_plural_fn(int32_t num)
{



    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t ko_lang = {
    .locale_name = "ko",
    .singulars = ko_singulars,

    .locale_plural_fn = ko_plural_fn
};

const static lv_i18n_phrase_t ru_singulars[] = {
    {"Address", "Адрес"},
    {"Primary_Address", "Основной адрес"},
    {"Quit", "Выйти"},
    {"Sub_Address", "Подадрес"},
    {"Undo", "отменить"},
    {"about_info_battery_voltage", "Напряжение батареи"},
    {"about_info_device_uid", "IOD устройства"},
    {"about_info_export_file_name", "прошивкой Название:"},
    {"about_info_export_log", "Выгрузка логов"},
    {"about_info_export_to_sdcard", "Выгрузить логи на MicroSD"},
    {"about_info_fingerprint_firmware_version", "Отпечаток версии прошивки"},
    {"about_info_firmware_version", "Версия прошивки"},
    {"about_info_firmware_version_head", "Прошивка"},
    {"about_info_result_export_failed", "Ошибка выгрузки"},
    {"about_info_result_export_failed_desc_no_sdcard", "Убедитесь, что вы вставили MicroSD карту, отформатированную в FAT32."},
    {"about_info_result_export_failed_desc_no_space", "Убедитесь, что на MicroSD карте достаточно памяти."},
    {"about_info_result_export_successful", "Успешная выгрузка"},
    {"about_info_result_export_successful_desc", "Системные логи были успешно выгружены на MicroSD карту."},
    {"about_info_serial_number", "Серийный номер"},
    {"about_info_verify_checksum_desc", "Проверьте, соответствует ли приведенная выше информация веб-странице. Если он не совпадает, это означает, что ваша прошивка могла быть подделана. Прекратите использование устройства."},
    {"about_info_verify_checksum_text", "Контрольная сумма"},
    {"about_info_verify_checksum_title", "Контрольная сумма"},
    {"about_info_verify_firmware_desc", "Это расширенная функция, позволяющая разработчикам проверить, соответствует ли прошивка устройства Forgebox той версии, исходный код которой мы предоставили в открытом доступе"},
    {"about_info_verify_firmware_step1", "Перейдите в репозиторий Forgebox на GitHub с открытым исходным кодом и следуйте инструкциям, чтобы собрать прошивку и получить контрольную сумму"},
    {"about_info_verify_firmware_step2", "Нажмите кнопку #F5870A Контрольная# #F5870A сумма# рядом с загрузкой прошивки"},
    {"about_info_verify_firmware_step3", "Нажмите кнопку #F5870A Показать контрольную# #F5870A сумму# ниже и сравните информацию, отображаемую на веб-странице и на вашем устройстве"},
    {"about_info_verify_source_code_title", "Проверка исходного кода"},
    {"about_keystone_discord", "Discord"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "Telegram"},
    {"about_keystone_telegram_url", "t.me/keystonewallet"},
    {"about_keystone_title", "О Forgebox"},
    {"about_keystone_twitter", "Twitter"},
    {"about_keystone_twitter_url", "twitter.com/ForgeboxWallet"},
    {"about_keystone_website", "Сайт"},
    {"about_keystone_website_url", "keyst.one"},
    {"about_terms_contact_us", "Связь с нами"},
    {"about_terms_contact_us_desc", "Если у вас есть какие-либо вопросы или проблемы, напишите нам по адресу support@keyst.one."},
    {"about_terms_desc", "Чтобы получить доступ к полной версии УСЛОВИЙ ИСПОЛЬЗОВАНИЯ, перейдите по ссылке:"},
    {"about_terms_disclaimers", "ОТКАЗ ОТ ОТВЕТСТВЕННОСТИ"},
    {"about_terms_disclaimers_desc", "Предоставленная информация не является финансовой рекомендацией. Прежде чем принимать какие-либо решения, обратитесь за профессиональной консультацией."},
    {"about_terms_discontinuance_service", "Прекращение обслуживания"},
    {"about_terms_discontinuance_service_desc", "Мы можем изменить или прекратить предоставление наших услуг. Не забудьте создать резервную копию сид фразы для доступа к вашим криптовалютам."},
    {"about_terms_eligibility", "Ограничение"},
    {"about_terms_eligibility_desc", "Для доступа и к нашим продуктам и услугам вам должно быть 18 лет или больше."},
    {"about_terms_indemnity", "Условия использования"},
    {"about_terms_law", "Применимое право и разрешение споров"},
    {"about_terms_law_desc", "Условия регулируются законодательством Гонконга, и любые споры должны быть поданы в течение одного года."},
    {"about_terms_modification", "Изменения настоящих условий"},
    {"about_terms_modification_desc", "Мы оставляем за собой право изменять настоящие Условия по своему усмотрению."},
    {"about_terms_no_sensitive_information", "Конфиденциальная информация"},
    {"about_terms_no_sensitive_information_desc", "Мы не храним вашу конфиденциальную информацию, такую как пароли или сид фразы. Только вы ответственны за их хранение."},
    {"about_terms_ownership", "Ответственность"},
    {"about_terms_ownership_desc", "Вы несете ответственность за свои действия при использовании Продуктов и Услуг."},
    {"about_terms_product_and_services", "Продукты и услуги Forgebox"},
    {"about_terms_product_and_services_desc", "Наш аппаратный кошелек безопасно управляет криптовалютами."},
    {"about_terms_prohibited_conduct", "Права собственности"},
    {"about_terms_prohibited_product_desc", "Наши Продукты и Услуги защищены законами об интеллектуальной собственности."},
    {"about_terms_risks", "Риски"},
    {"about_terms_risks_desc", "Помните о рисках, связанных с криптовалютами и уязвимостями технологий."},
    {"about_terms_subtitle", "Условия использования Forgebox"},
    {"about_terms_title", "Условия использования"},
    {"about_terms_website_url", "https://keyst.one/terms"},
    {"calculat_modal_title", "Расчет"},
    {"device_info_title", "Об устройстве"},
    {"firmware_update_deny_desc", "Необходимо разблокировать устройство для обновления прошивки."},
    {"firmware_update_deny_input_password", "Введите пароль"},
    {"firmware_update_deny_title", "Разблокируйте устройство"},
    {"firmware_update_desc", "Чтобы получить больше возможностей обновите прошивку до последней версии."},
    {"firmware_update_desc1", "Убедитесь, что у вашего Forgebox осталось не менее 40% заряда батареи."},
    {"firmware_update_desc2", "Перейдите на страницу обновления прошивки Forgebox с помощью компьютера или смартфона."},
    {"firmware_update_no_upgradable_firmware_desc", "Обновляемой прошивки не обнаружено"},
    {"firmware_update_no_upgradable_firmware_title", "Обновляемой прошивки не обнаружено"},
    {"firmware_update_sd_checksum_desc", "#F5870A Показать контрольную сумму#"},
    {"firmware_update_sd_checksum_done", "Контрольная сумма:\r\n"},
    {"firmware_update_sd_checksum_fmt", "#F5870A Показать контрольную сумму(%d%%)#"},
    {"firmware_update_sd_checksum_notice", "Это дополнительная функция, предназначенная для дальнейшего повышения безопасности. Сравните следующую контрольную сумму с контрольной суммой вашего загружаемого пакета на официальном сайте и убедитесь, что они совпадают"},
    {"firmware_update_sd_copying_desc", "Не доставайте MicroSD карту во время загрузки прошивки."},
    {"firmware_update_sd_copying_title", "Старт обновления"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "Перенесите файл с прошивкой (forgebox.bin) на Forgebox используя MicroSD карту с форматированием FAT32."},
    {"firmware_update_sd_desc4", "Нажмите кнопку #F5870A Обновить# ниже, чтобы начать процесс."},
    {"firmware_update_sd_dialog_desc", "Вы хотите обновить прошивку вашего устройства до новой версии?"},
    {"firmware_update_sd_dialog_title", "Обнаружено новое программное обеспечение"},
    {"firmware_update_sd_failed_access_desc", "Убедитесь, что вы правильно вставили MicroSD карту"},
    {"firmware_update_sd_failed_access_title", "MicroSD карта не обнаружена"},
    {"firmware_update_sd_title", "MicroSD обновление"},
    {"firmware_update_title", "Обновление прошивки"},
    {"firmware_update_updating_desc", "Это займет около 10 минут"},
    {"firmware_update_usb_connect_info_desc", "После подключения внешнее устройство получит разрешение на передачу данных на Forgebox."},
    {"firmware_update_usb_connect_info_title", "Подключить устройство?"},
    {"firmware_update_usb_desc3", "Подключите Forgebox к компьютеру с помощью кабеля USB-C."},
    {"firmware_update_usb_desc4", "Нажмите кнопку #F5870A Установить обновление# на веб-странице и следуйте инструкциям по установке."},
    {"firmware_update_usb_desc5", "Не отключайте USB кабель во время процесса установки."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "Обновление прошивки"},
    {"firmware_update_usb_title", "USB обновление"},
    {"firmware_update_usb_title2", "Внимание"},
    {"firmware_update_usb_updating_hint", "Не отключайте USB кабель во время процесса установки."},
    {"firmware_update_verify_firmware_qr_link", "ForgeboxHQ/forgebox-firmware/docs/verify.md"},
    {"firmware_update_via_sd", "Через MicroSD карту"},
    {"firmware_update_via_usb", "Через USB кабель"},
    {"illustrate_supplement", "."},
    {"language_desc", "Выберите язык"},
    {"language_little_title", "Язык"},
    {"language_option", "Русский язык"},
    {"language_title", "Язык"},
    {"password_error_not_match", "Неверный код-пароль"},
    {"password_error_too_short", "Пароль должен состоять минимум из 6 символов"},
    {"sdcard_format_confirm", "Формат"},
    {"sdcard_format_desc", "Ваша карта microSD будет отформатирована в формате FAT32, при этом все файлы будут удалены. Пожалуйста, перед форматированием создайте резервные копии необходимых файлов."},
    {"sdcard_format_failed_desc", "Не удалось выполнить форматирование, замените карту microSD или отформатируйте ее на компьютере."},
    {"sdcard_format_failed_title", "Незавершено форматирование"},
    {"sdcard_format_subtitle", "Формат карты microSD"},
    {"sdcard_format_success_desc", "Карта microSD была успешно отформатирована в FAT32."},
    {"sdcard_format_success_title", "Форматирование завершено"},
    {"sdcard_format_text", "Формат карты microSD"},
    {"sdcard_formating", "в процессе форматирования"},
    {"sdcard_formating_desc", "Не извлекайте карту microSD во время выполнения форматирования."},
    {"update_success", "Обновление завершено"},
    {"usb_connection_desc", "Если этот режим включен, USB-порт можно использовать только для зарядки аккумулятора."},
    {"usb_connection_subtitle", "Режим воздушного зазора"},
    {"usb_connection_title", "Соединение"},
    {"verification_code_desc", "Введите этот код на официальном сайте Forgebox, чтобы проверить подлинность устройства."},
    {"verification_code_failed_desc", "Ваше устройство могло быть скомпрометировано, что представляет угрозу для ваших конфиденциальных данных и цифровых активов.В целях безопасности мы рекомендуем удалить все личные данные и немедленно обратиться в службу поддержки Forgebox за помощью."},
    {"verification_code_failed_title", "Осторожно! Ваше устройство скомпрометировано!"},
    {"verification_code_title", "Код проверки"},
    {"verification_success", "Проверено"},
    {"verify_cont1", "Посетите наш веб-сайт и нажмите на кнопку #F5870A Начать проверку#."},
    {"verify_cont2", "Отсканируйте QR-код, отображаемый на веб-сайте, чтобы получить код подтверждения устройства."},
    {"verify_cont3", "Введите код на веб-сайте, чтобы завершить проверку устройства на подлинность."},
    {"verify_desc", "Этот процесс подтвердит подлинность устройства Forgebox и его прошивки."},
    {"verify_firmware", "Провека прошивки"},
    {"verify_modal_desc", "Вычисление кода авторизации..."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "Сканировать QR-код"},
    {"verify_title", "Проверка устройства"},
    {"verify_title_text", "Проверка устройства"},
    {NULL, NULL} // End mark
};



static uint8_t ru_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);
    uint32_t v = op_v(n); UNUSED(v);
    uint32_t i = op_i(n); UNUSED(i);
    uint32_t i10 = i % 10;
    uint32_t i100 = i % 100;
    if ((v == 0 && i10 == 1 && i100 != 11)) return LV_I18N_PLURAL_TYPE_ONE;
    if ((v == 0 && (2 <= i10 && i10 <= 4) && (!(12 <= i100 && i100 <= 14)))) return LV_I18N_PLURAL_TYPE_FEW;
    if ((v == 0 && i10 == 0) || (v == 0 && (5 <= i10 && i10 <= 9)) || (v == 0 && (11 <= i100 && i100 <= 14))) return LV_I18N_PLURAL_TYPE_MANY;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t ru_lang = {
    .locale_name = "ru",
    .singulars = ru_singulars,

    .locale_plural_fn = ru_plural_fn
};

const static lv_i18n_phrase_t zh_cn_singulars[] = {
    {"Address", "地址"},
    {"Primary_Address", "主地址"},
    {"Quit", "退出"},
    {"Sub_Address", "子地址"},
    {"Undo", "撤销"},
    {"about_info_battery_voltage", "电池电压"},
    {"about_info_device_uid", "设备UID"},
    {"about_info_export_file_name", "文件名"},
    {"about_info_export_log", "导出系统日志"},
    {"about_info_export_to_sdcard", "将日志导出到 microSD卡"},
    {"about_info_fingerprint_firmware_version", "指纹固件版本"},
    {"about_info_firmware_version", "固件版本"},
    {"about_info_firmware_version_head", "固件"},
    {"about_info_result_export_failed", "导出失败"},
    {"about_info_result_export_failed_desc_no_sdcard", "请确保您插入了 FAT32 格式的 microSD 卡."},
    {"about_info_result_export_failed_desc_no_space", "请确保您的 microSD 卡具有足够的内存."},
    {"about_info_result_export_successful", "导出成功"},
    {"about_info_result_export_successful_desc", "您的系统日志已成功导出到 microSD 卡."},
    {"about_info_serial_number", "序列号"},
    {"about_info_verify_checksum_desc", "如果以上信息与您自行计算的 mh1903.bin 文件的校验和匹配,则意味着我们的开源固件与设备中的固件一致."},
    {"about_info_verify_checksum_text", "校验和"},
    {"about_info_verify_checksum_title", "校验和"},
    {"about_info_verify_firmware_desc", "这是一个高级功能,用于开发人员验证在Forgebox设备上运行的固件是否与我们开源的固件相一致."},
    {"about_info_verify_firmware_step1", "进入 Forgebox 的开源 GitHub 页面,并按照说明进行构建固件并计算对应校验和."},
    {"about_info_verify_firmware_step2", "点击固件页面中,\"固件下载\"按钮旁的 #F5870A Checksum#."},
    {"about_info_verify_firmware_step3", "点击下面的 #F5870A 展示校验和# 按钮,然后将第二步的校验和与之进行对比."},
    {"about_info_verify_source_code_title", "验证源代码"},
    {"about_keystone_discord", "Discord"},
    {"about_keystone_discord_url", "keyst.one/discord"},
    {"about_keystone_telegram", "Telegram"},
    {"about_keystone_telegram_url", "T.Me/ForgeboxWallet"},
    {"about_keystone_title", "关于 Forgebox"},
    {"about_keystone_twitter", "Twitter"},
    {"about_keystone_twitter_url", "Twitter.com/ForgeboxWallet"},
    {"about_keystone_website", "网站"},
    {"about_keystone_website_url", "keyst.one"},
    {"about_terms_contact_us", "联系我们"},
    {"about_terms_contact_us_desc", "如果您有任何疑问或疑虑,请给我们发送电子邮件至support@keyst.one."},
    {"about_terms_desc", "要访问使用条款的完整版本,请访问以下链接:"},
    {"about_terms_disclaimers", "免责声明"},
    {"about_terms_disclaimers_desc", "提供的信息不是财务建议.在做出任何决定之前,请寻求专业建议."},
    {"about_terms_discontinuance_service", "服务中断"},
    {"about_terms_discontinuance_service_desc", "我们可能会修改或停止我们的服务.请务必备份您的助记词以访问您的加密货币."},
    {"about_terms_eligibility", "合格"},
    {"about_terms_eligibility_desc", "您必须年满18岁才能访问和使用我们的产品或服务."},
    {"about_terms_indemnity", "赔款"},
    {"about_terms_law", "管理法律和争议解决"},
    {"about_terms_law_desc", "该条款受香港法律管辖,任何争议都必须在一年内提出."},
    {"about_terms_modification", "修改这些术语"},
    {"about_terms_modification_desc", "我们保留酌情决定更改这些条款的权利."},
    {"about_terms_no_sensitive_information", "不检索敏感信息"},
    {"about_terms_no_sensitive_information_desc", "我们不存储您的敏感信息,例如密码或助记词.保持您的凭据安全."},
    {"about_terms_ownership", "所有权和专有权利"},
    {"about_terms_ownership_desc", "在使用产品和服务时,您应对自己的行动负责."},
    {"about_terms_product_and_services", "Forgebox产品和服务"},
    {"about_terms_product_and_services_desc", "我们的硬件钱包安全地管理加密货币."},
    {"about_terms_prohibited_conduct", "禁止行为"},
    {"about_terms_prohibited_product_desc", "我们的产品和服务受知识产权法保护."},
    {"about_terms_risks", "风险"},
    {"about_terms_risks_desc", "请注意与加密货币和技术漏洞相关的风险."},
    {"about_terms_subtitle", "基石使用条款"},
    {"about_terms_title", "使用条款"},
    {"about_terms_website_url", "https://keyst.one/terms"},
    {"calculat_modal_title", "计算中..."},
    {"device_info_title", "设备信息"},
    {"firmware_update_deny_desc", "您需要解锁设备以升级固件版本."},
    {"firmware_update_deny_input_password", "输入密码"},
    {"firmware_update_deny_title", "需要解锁设备"},
    {"firmware_update_desc", "如需体验最新功能,请下载最新固件!"},
    {"firmware_update_desc1", "请确保您的设备电量高于 40%."},
    {"firmware_update_desc2", "使用计算机或手机进入 Forgebox 的固件更新页面."},
    {"firmware_update_no_upgradable_firmware_desc", "设备当前版本已高于或等于 microSD 卡上的固件版本."},
    {"firmware_update_no_upgradable_firmware_title", "未检测到可升级固件"},
    {"firmware_update_sd_checksum_desc", "#F5870A 展示校验和#"},
    {"firmware_update_sd_checksum_done", "校验和\r\n"},
    {"firmware_update_sd_checksum_fmt", "#F5870A 展示校验和 (%d%%)#"},
    {"firmware_update_sd_checksum_notice", "这是一个可选的功能,可以进一步提高安全性.将以下校验和在官方网站上的下载软件包的校验和签到,请确保它们保持一致."},
    {"firmware_update_sd_copying_desc", "升级过程中,请勿移除 microSD 卡."},
    {"firmware_update_sd_copying_title", "开始更新"},
    {"firmware_update_sd_desc2_link", "https://keyst.one/firmware"},
    {"firmware_update_sd_desc3", "将固件(forgebox.bin)下载到 FAT32 格式的 microSD 卡内,插入到设备中."},
    {"firmware_update_sd_desc4", "点击下方的#F5870A 更新#按钮,开始升级."},
    {"firmware_update_sd_dialog_desc", "您想将设备的固件更新到新版本吗?"},
    {"firmware_update_sd_dialog_title", "检测到新固件"},
    {"firmware_update_sd_failed_access_desc", "请确保您已正确插入了microSD卡."},
    {"firmware_update_sd_failed_access_title", "未检测到 microSD 卡"},
    {"firmware_update_sd_title", "MicroSD更新"},
    {"firmware_update_title", "固件升级"},
    {"firmware_update_updating_desc", "大约需要 10 分钟"},
    {"firmware_update_usb_connect_info_desc", "连接后,外部设备将获得将数据传输到 Forgebox 的权限."},
    {"firmware_update_usb_connect_info_title", "连接到此设备?"},
    {"firmware_update_usb_desc3", "使用 USB-C 线缆将设备连接至计算机."},
    {"firmware_update_usb_desc4", "单击 \"WebUSB 固件升级\" 页面上的#F5870A 开始连接#按钮,然后根据提示安装最新的固件."},
    {"firmware_update_usb_desc5", "升级中,请保持 USB 连接."},
    {"firmware_update_usb_qr_link", "https://keyst.one/webusb"},
    {"firmware_update_usb_qr_title", "固件升级"},
    {"firmware_update_usb_title", "USB 更新"},
    {"firmware_update_usb_title2", "警告"},
    {"firmware_update_usb_updating_hint", "升级中,请保持 USB 连接."},
    {"firmware_update_verify_firmware_qr_link", "ForgeboxHQ/forgebox-firmware/docs/verify.md"},
    {"firmware_update_via_sd", "MicroSD卡"},
    {"firmware_update_via_usb", "USB"},
    {"illustrate_supplement", "小秒天"},
    {"language_desc", "选择你的语言"},
    {"language_little_title", "语言"},
    {"language_option", "简体中文"},
    {"language_title", "语言"},
    {"password_error_not_match", "密码不匹配"},
    {"password_error_too_short", "密码必须至少 6 个字符"},
    {"sdcard_format_confirm", "格式化"},
    {"sdcard_format_desc", "您的 MicroSD 卡将被格式化为 FAT32,并被擦除所有文件.请在格式化之前备份重要文件."},
    {"sdcard_format_failed_desc", "格式化失败,请更换 MicroSD 卡或在计算机上进行格式化."},
    {"sdcard_format_failed_title", "格式化失败"},
    {"sdcard_format_subtitle", "格式化 MicroSD 卡"},
    {"sdcard_format_success_desc", "MicroSD 卡已成功格式化为 FAT32."},
    {"sdcard_format_success_title", "格式化完成"},
    {"sdcard_format_text", "格式化 MicroSD 卡"},
    {"sdcard_formating", "正在格式化"},
    {"sdcard_formating_desc", "在格式化过程中请勿移除 MicroSD 卡."},
    {"update_success", "升级成功"},
    {"usb_connection_desc", "启用气隙模式后,USB接口将仅用于充电"},
    {"usb_connection_subtitle", "气隙模式"},
    {"usb_connection_title", "连接方式"},
    {"verification_code_desc", "在\"设备验证\"页面上输入验证码,检查您的设备是否安全."},
    {"verification_code_failed_desc", "您的设备存在潜在篡改可能,这将会对您的隐私数据和数字资产造成威胁.为了您的资产安全,我们建议您删除所有个人数据,并立即与Forgebox支持团队联系以寻求帮助."},
    {"verification_code_failed_title", "设备异常!"},
    {"verification_code_title", "验证码"},
    {"verification_success", "验证成功"},
    {"verify_cont1", "访问我们的网站,然后单击#F5870A 设备验证#按钮."},
    {"verify_cont2", "使用设备扫描\"设备验证\"页面上显示的二维码后,会在设备上显示验证码."},
    {"verify_cont3", "在\"设备验证\"页面上输入验证码,检查您的设备是否被篡改."},
    {"verify_desc", "通过设备验证功能,可以校验您的设备是否被篡改过;也可以用来校验固件的完整性."},
    {"verify_firmware", "验证固件"},
    {"verify_modal_desc", "获取验证码..."},
    {"verify_qr_link", "https://keyst.one/verify"},
    {"verify_scan_qr_code", "扫描二维码"},
    {"verify_title", "设备验证"},
    {"verify_title_text", "设备验证"},
    {NULL, NULL} // End mark
};



static uint8_t zh_cn_plural_fn(int32_t num)
{



    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t zh_cn_lang = {
    .locale_name = "zh-CN",
    .singulars = zh_cn_singulars,

    .locale_plural_fn = zh_cn_plural_fn
};

const lv_i18n_language_pack_t lv_i18n_language_pack[] = {
    &en_lang,
    &de_lang,
    &es_lang,
    &ja_lang,
    &ko_lang,
    &ru_lang,
    &zh_cn_lang,
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

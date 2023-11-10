#include "eapdu_protocol_parser.h"
#include "librust_c.h"
// FIXME: It is not reasonable to include "gui_wallet" here because this is a USB thread and "gui" generally represents a UI thread.
// Consider finding a way to separate these concerns to maintain thread safety and better organize the code.
#include "gui_wallet.h"

void *ExportAddressService(EAPDURequestPayload_t payload);
void ExportAddressApprove();
void ExportAddressReject();

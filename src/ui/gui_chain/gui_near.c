#include "rust.h"
#include "keystore.h"
#include "user_memory.h"
#include "gui_chain.h"
#include "screen_manager.h"
#include "keystore.h"
#include "account_manager.h"
#include "secret_cache.h"

char *GuiGetNearPubkey(uint8_t pathType, uint16_t index)
{
    switch (pathType)
    {
    case 0:
        return GetCurrentAccountPublicKey(XPUB_TYPE_NEAR_STANDARD_0);
    case 1:
        return GetCurrentAccountPublicKey(XPUB_TYPE_NEAR_LEDGER_LIVE_0 + index);
    default:
        printf("GuiGetNearPubkey: pathType = %d\n is not supported", pathType);
        return "";
    }
}

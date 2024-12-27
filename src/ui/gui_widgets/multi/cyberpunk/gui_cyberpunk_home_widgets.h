#ifdef CYBERPUNK_VERSION
#ifndef _GUI_CYBERPUNK_HOME_WIDGETS_H
#define _GUI_CYBERPUNK_HOME_WIDGETS_H

#define HOME_WIDGETS_SURPLUS_CARD_ENUM     HOME_WALLET_CARD_ZEC, \
    HOME_WALLET_CARD_MONERO

#define HOME_WALLET_STATE_SURPLUS          {HOME_WALLET_CARD_ZEC, false, "ZEC", true}, \
    {HOME_WALLET_CARD_MONERO, false, "XMR", true}

#define HOME_WALLET_CARD_SURPLUS           { \
        .index = HOME_WALLET_CARD_ZEC, \
        .coin = "ZEC", \
        .chain = "Zcash", \
        .icon = &coinZec, \
    }, \
    { \
        .index = HOME_WALLET_CARD_MONERO, \
        .coin = "XMR", \
        .chain = "Monero", \
        .icon = &coinXmr, \
    }

#define HOME_WALLET_HIDDEN_CARD_ENUM HOME_WALLET_CARD_BUTT

#endif /* _GUI_CYBERPUNK_HOME_WIDGETS_H */
#endif
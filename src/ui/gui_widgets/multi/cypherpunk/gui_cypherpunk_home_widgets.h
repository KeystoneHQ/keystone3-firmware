#ifdef CYPHERPUNK_VERSION
#ifndef _GUI_CYPHERPUNK_HOME_WIDGETS_H
#define _GUI_CYPHERPUNK_HOME_WIDGETS_H

#define HOME_WIDGETS_SURPLUS_CARD_ENUM     HOME_WALLET_CARD_ZEC, \
    HOME_WALLET_CARD_MONERO

#define HOME_WALLET_STATE_SURPLUS          {HOME_WALLET_CARD_ZEC, true, "ZEC", true, MainNet}, \
    {HOME_WALLET_CARD_MONERO, true, "XMR", true, MainNet}

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

#endif /* _GUI_CYPHERPUNK_HOME_WIDGETS_H */
#endif

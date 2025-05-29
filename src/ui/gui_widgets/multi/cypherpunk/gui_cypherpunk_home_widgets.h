#ifdef CYPHERPUNK_VERSION
#ifndef _GUI_CYPHERPUNK_HOME_WIDGETS_H
#define _GUI_CYPHERPUNK_HOME_WIDGETS_H

#define HOME_WIDGETS_SURPLUS_CARD_ENUM     HOME_WALLET_CARD_ZEC, \
    HOME_WALLET_CARD_MONERO, \
    HOME_WALLET_CARD_ERG

#define HOME_WALLET_STATE_SURPLUS          {HOME_WALLET_CARD_ZEC, true, "ZEC", true}, \
    {HOME_WALLET_CARD_MONERO, true, "XMR", true}, \
    {HOME_WALLET_CARD_ERG, false, "ERG", true}


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
    }, \
    { \
        .index = HOME_WALLET_CARD_ERG, \
        .coin = "ERG", \
        .chain = "Ergo", \
        .icon = &coinErg, \
    }

#endif /* _GUI_CYPHERPUNK_HOME_WIDGETS_H */
#endif
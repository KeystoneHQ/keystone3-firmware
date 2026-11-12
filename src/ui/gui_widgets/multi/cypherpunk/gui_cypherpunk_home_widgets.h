#ifdef CYPHERPUNK_VERSION
#ifndef _GUI_CYPHERPUNK_HOME_WIDGETS_H
#define _GUI_CYPHERPUNK_HOME_WIDGETS_H

#define HOME_WIDGETS_SURPLUS_CARD_ENUM     HOME_WALLET_CARD_ZEC, \
    HOME_WALLET_CARD_MONERO

/// There is no GUI for network selection yet: change `MainNet` to `TestNet` in
/// the `HOME_WALLET_CARD_ZEC` entry to run Zcash against testnet. The Bitcoin
/// and Monero entries are mainnet-only, and changing theirs has no effect.
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
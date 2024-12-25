 #ifdef GENERAL_VERSION
#ifndef _GUI_GENERAL_HOME_WIDGETS_H
#define _GUI_GENERAL_HOME_WIDGETS_H

#define HOME_WIDGETS_SURPLUS_CARD_ENUM         HOME_WALLET_CARD_ETH, \
    HOME_WALLET_CARD_ZEC, \
    HOME_WALLET_CARD_SOL, \
    HOME_WALLET_CARD_BNB, \
    HOME_WALLET_CARD_HNT, \
    HOME_WALLET_CARD_XRP, \
    HOME_WALLET_CARD_ADA, \
    HOME_WALLET_CARD_TON, \
    HOME_WALLET_CARD_DOT, \
    HOME_WALLET_CARD_TRX, \
    HOME_WALLET_CARD_LTC, \
    HOME_WALLET_CARD_BCH, \
    HOME_WALLET_CARD_APT, \
    HOME_WALLET_CARD_SUI, \
    HOME_WALLET_CARD_DASH, \
    HOME_WALLET_CARD_ARWEAVE, \
    HOME_WALLET_CARD_XLM, \
    HOME_WALLET_CARD_COSMOS, \
    HOME_WALLET_CARD_TIA, \
    HOME_WALLET_CARD_NTRN, \
    HOME_WALLET_CARD_DYM, \
    HOME_WALLET_CARD_OSMO, \
    HOME_WALLET_CARD_INJ, \
    HOME_WALLET_CARD_ATOM, \
    HOME_WALLET_CARD_CRO, \
    HOME_WALLET_CARD_RUNE, \
    HOME_WALLET_CARD_KAVA, \
    HOME_WALLET_CARD_LUNC, \
    HOME_WALLET_CARD_AXL, \
    HOME_WALLET_CARD_LUNA, \
    HOME_WALLET_CARD_AKT, \
    HOME_WALLET_CARD_STRD, \
    HOME_WALLET_CARD_SCRT, \
    HOME_WALLET_CARD_BLD, \
    HOME_WALLET_CARD_CTK, \
    HOME_WALLET_CARD_EVMOS, \
    HOME_WALLET_CARD_STARS, \
    HOME_WALLET_CARD_XPRT, \
    HOME_WALLET_CARD_SOMM, \
    HOME_WALLET_CARD_JUNO, \
    HOME_WALLET_CARD_IRIS, \
    HOME_WALLET_CARD_DVPN, \
    HOME_WALLET_CARD_ROWAN, \
    HOME_WALLET_CARD_REGEN, \
    HOME_WALLET_CARD_BOOT, \
    HOME_WALLET_CARD_GRAV, \
    HOME_WALLET_CARD_IXO, \
    HOME_WALLET_CARD_NGM, \
    HOME_WALLET_CARD_IOV, \
    HOME_WALLET_CARD_UMEE, \
    HOME_WALLET_CARD_QCK, \
    HOME_WALLET_CARD_TGD
    // last one cant not with comma

#define HOME_WALLET_STATE_SURPLUS          {HOME_WALLET_CARD_ETH, false, "ETH", true}, \
    {HOME_WALLET_CARD_SOL, false, "SOL", true}, \
    {HOME_WALLET_CARD_BNB, false, "BNB", false}, \
    {HOME_WALLET_CARD_HNT, false, "HNT", true}, \
    {HOME_WALLET_CARD_XRP, false, "XRP", true}, \
    {HOME_WALLET_CARD_ADA, false, "ADA", true}, \
    {HOME_WALLET_CARD_TON, false, "TON", false}, \
    {HOME_WALLET_CARD_DOT, false, "DOT", false}, \
    {HOME_WALLET_CARD_TRX, false, "TRX", true}, \
    {HOME_WALLET_CARD_LTC, false, "LTC", true}, \
    {HOME_WALLET_CARD_BCH, false, "BCH", true}, \
    {HOME_WALLET_CARD_APT, false, "APT", true}, \
    {HOME_WALLET_CARD_SUI, false, "SUI", true}, \
    {HOME_WALLET_CARD_DASH, false, "DASH", true}, \
    {HOME_WALLET_CARD_ARWEAVE, false, "AR", true}, \
    {HOME_WALLET_CARD_XLM, false, "XLM", true}, \
    {HOME_WALLET_CARD_COSMOS, false, "Cosmos Eco", true}, \
    {HOME_WALLET_CARD_TIA, false, "TIA", true}, \
    {HOME_WALLET_CARD_NTRN, false, "NTRN", true}, \
    {HOME_WALLET_CARD_DYM, false, "DYM", true}, \
    {HOME_WALLET_CARD_OSMO, false, "OSMO", true}, \
    {HOME_WALLET_CARD_INJ, false, "INJ", true}, \
    {HOME_WALLET_CARD_ATOM, false, "ATOM", true}, \
    {HOME_WALLET_CARD_CRO, false, "CRO", true}, \
    {HOME_WALLET_CARD_RUNE, false, "RUNE", true}, \
    {HOME_WALLET_CARD_KAVA, false, "KAVA", true}, \
    {HOME_WALLET_CARD_LUNC, false, "LUNC", true}, \
    {HOME_WALLET_CARD_AXL, false, "AXL", true}, \
    {HOME_WALLET_CARD_LUNA, false, "LUNA", true}, \
    {HOME_WALLET_CARD_AKT, false, "AKT", true}, \
    {HOME_WALLET_CARD_STRD, false, "STRD", true}, \
    {HOME_WALLET_CARD_SCRT, false, "SCRT", true}, \
    {HOME_WALLET_CARD_BLD, false, "BLD", true}, \
    {HOME_WALLET_CARD_CTK, false, "CTK", true}, \
    {HOME_WALLET_CARD_EVMOS, false, "EVMOS", true}, \
    {HOME_WALLET_CARD_STARS, false, "STARS", true}, \
    {HOME_WALLET_CARD_XPRT, false, "XPRT", true}, \
    {HOME_WALLET_CARD_SOMM, false, "SOMM", true}, \
    {HOME_WALLET_CARD_JUNO, false, "JUNO", true}, \
    {HOME_WALLET_CARD_IRIS, false, "IRIS", true}, \
    {HOME_WALLET_CARD_DVPN, false, "DVPN", true}, \
    {HOME_WALLET_CARD_ROWAN, false, "ROWAN", true}, \
    {HOME_WALLET_CARD_REGEN, false, "REGEN", true}, \
    {HOME_WALLET_CARD_BOOT, false, "BOOT", true}, \
    {HOME_WALLET_CARD_GRAV, false, "GRAV", true}, \
    {HOME_WALLET_CARD_IXO, false, "IXO", true}, \
    {HOME_WALLET_CARD_NGM, false, "NGM", true}, \
    {HOME_WALLET_CARD_IOV, false, "IOV", true}, \
    {HOME_WALLET_CARD_UMEE, false, "UMEE", true}, \
    {HOME_WALLET_CARD_QCK, false, "QCK", true}, \
    {HOME_WALLET_CARD_TGD, false, "TGD", true}
    // last one cant not with comma

#define HOME_WALLET_CARD_SURPLUS           { \
        .index = HOME_WALLET_CARD_ETH, \
        .coin = "ETH", \
        .chain = "Ethereum", \
        .icon = &coinEth, \
    }, \
    { \
        .index = HOME_WALLET_CARD_SOL, \
        .coin = "SOL", \
        .chain = "Solana", \
        .icon = &coinSol, \
    }, \
    { \
        .index = HOME_WALLET_CARD_BNB, \
        .coin = "BNB", \
        .chain = "Binance", \
        .icon = &coinBnb, \
    }, \
    { \
        .index = HOME_WALLET_CARD_HNT, \
        .coin = "HNT", \
        .chain = "Helium", \
        .icon = &coinHelium, \
    }, \
    { \
        .index = HOME_WALLET_CARD_XRP, \
        .coin = "XRP", \
        .chain = "Ripple", \
        .icon = &coinXrp, \
    }, \
    { \
        .index = HOME_WALLET_CARD_ADA, \
        .coin = "ADA", \
        .chain = "Cardano", \
        .icon = &coinAda, \
    }, \
    { \
        .index = HOME_WALLET_CARD_TON, \
        .coin = "TON", \
        .chain = "The Open Network", \
        .icon = &coinTon, \
    }, \
    { \
        .index = HOME_WALLET_CARD_DOT, \
        .coin = "DOT", \
        .chain = "Polkadot", \
        .icon = &coinDot, \
    }, \
    { \
        .index = HOME_WALLET_CARD_TRX, \
        .coin = "TRX", \
        .chain = "TRON", \
        .icon = &coinTrx, \
    }, \
    { \
        .index = HOME_WALLET_CARD_LTC, \
        .coin = "LTC", \
        .chain = "Litecoin", \
        .icon = &coinLtc, \
    }, \
    { \
        .index = HOME_WALLET_CARD_BCH, \
        .coin = "BCH", \
        .chain = "Bitcoin Cash", \
        .icon = &coinBch, \
    }, \
    { \
        .index = HOME_WALLET_CARD_APT, \
        .coin = "APT", \
        .chain = "Aptos", \
        .icon = &coinApt, \
    }, \
    { \
        .index = HOME_WALLET_CARD_SUI, \
        .coin = "SUI", \
        .chain = "Sui", \
        .icon = &coinSui, \
    }, \
    { \
        .index = HOME_WALLET_CARD_DASH, \
        .coin = "DASH", \
        .chain = "Dash", \
        .icon = &coinDash, \
    }, \
    { \
        .index = HOME_WALLET_CARD_ARWEAVE, \
        .coin = "AR", \
        .chain = "Arweave", \
        .icon = &coinAr, \
    }, \
    { \
        .index = HOME_WALLET_CARD_XLM, \
        .coin = "XLM", \
        .chain = "Stellar", \
        .icon = &coinXlm, \
    }, \
    { \
        .index = HOME_WALLET_CARD_COSMOS, \
        .coin = "Cosmos Eco", \
        .chain = "", \
        .icon = &coinCosmos, \
    }, \
    { \
        .index = HOME_WALLET_CARD_TIA, \
        .coin = "TIA", \
        .chain = "Celestia", \
        .icon = &coinTia, \
    }, \
    { \
        .index = HOME_WALLET_CARD_NTRN, \
        .coin = "NTRN", \
        .chain = "Neutron", \
        .icon = &coinNtrn, \
    }, \
    { \
        .index = HOME_WALLET_CARD_DYM, \
        .coin = "DYM", \
        .chain = "Dymension", \
        .icon = &coinDym, \
    }, \
    { \
        .index = HOME_WALLET_CARD_OSMO, \
        .coin = "OSMO", \
        .chain = "Osmosis", \
        .icon = &coinOsmo, \
    }, \
    { \
        .index = HOME_WALLET_CARD_INJ, \
        .coin = "INJ", \
        .chain = "Injective", \
        .icon = &coinInj, \
    }, \
    { \
        .index = HOME_WALLET_CARD_ATOM, \
        .coin = "ATOM", \
        .chain = "Cosmos Hub", \
        .icon = &coinAtom, \
    }, \
    { \
        .index = HOME_WALLET_CARD_CRO, \
        .coin = "CRO", \
        .chain = "Cronos POS chain", \
        .icon = &coinCro, \
    }, \
    { \
        .index = HOME_WALLET_CARD_RUNE, \
        .coin = "RUNE", \
        .chain = "THORChain", \
        .icon = &coinRune, \
    }, \
    { \
        .index = HOME_WALLET_CARD_KAVA, \
        .coin = "KAVA", \
        .chain = "Kava", \
        .icon = &coinKava, \
    }, \
    { \
        .index = HOME_WALLET_CARD_LUNC, \
        .coin = "LUNC", \
        .chain = "Terra Classic", \
        .icon = &coinLunc, \
    }, \
    { \
        .index = HOME_WALLET_CARD_AXL, \
        .coin = "AXL", \
        .chain = "Axelar", \
        .icon = &coinAxl, \
    }, \
    { \
        .index = HOME_WALLET_CARD_LUNA, \
        .coin = "LUNA", \
        .chain = "Terra", \
        .icon = &coinLuna, \
    }, \
    { \
        .index = HOME_WALLET_CARD_AKT, \
        .coin = "AKT", \
        .chain = "Akash", \
        .icon = &coinAkt, \
    }, \
    { \
        .index = HOME_WALLET_CARD_STRD, \
        .coin = "STRD", \
        .chain = "Stride", \
        .icon = &coinStrd, \
    }, \
    { \
        .index = HOME_WALLET_CARD_SCRT, \
        .coin = "SCRT", \
        .chain = "Secret Network", \
        .icon = &coinScrt, \
    }, \
    { \
        .index = HOME_WALLET_CARD_BLD, \
        .coin = "BLD", \
        .chain = "Agoric", \
        .icon = &coinBld, \
    }, \
    { \
        .index = HOME_WALLET_CARD_CTK, \
        .coin = "CTK", \
        .chain = "Shentu", \
        .icon = &coinCtk, \
    }, \
    { \
        .index = HOME_WALLET_CARD_EVMOS, \
        .coin = "EVMOS", \
        .chain = "Evmos", \
        .icon = &coinEvmos, \
    }, \
    { \
        .index = HOME_WALLET_CARD_STARS, \
        .coin = "STARS", \
        .chain = "Stargaze", \
        .icon = &coinStars, \
    }, \
    { \
        .index = HOME_WALLET_CARD_XPRT, \
        .coin = "XPRT", \
        .chain = "Persistence", \
        .icon = &coinXprt, \
    }, \
    { \
        .index = HOME_WALLET_CARD_SOMM, \
        .coin = "SOMM", \
        .chain = "Sommelier", \
        .icon = &coinSomm, \
    }, \
    { \
        .index = HOME_WALLET_CARD_JUNO, \
        .coin = "JUNO", \
        .chain = "Juno", \
        .icon = &coinJuno, \
    }, \
    { \
        .index = HOME_WALLET_CARD_IRIS, \
        .coin = "IRIS", \
        .chain = "IRISnet", \
        .icon = &coinIris, \
    }, \
    { \
        .index = HOME_WALLET_CARD_DVPN, \
        .coin = "DVPN", \
        .chain = "Sentinel", \
        .icon = &coinDvpn, \
    }, \
    { \
        .index = HOME_WALLET_CARD_ROWAN, \
        .coin = "ROWAN", \
        .chain = "Sifchain", \
        .icon = &coinRowan, \
    }, \
    { \
        .index = HOME_WALLET_CARD_REGEN, \
        .coin = "REGEN", \
        .chain = "Regen", \
        .icon = &coinRegen, \
    }, \
    { \
        .index = HOME_WALLET_CARD_BOOT, \
        .coin = "BOOT", \
        .chain = "Bostrom", \
        .icon = &coinBoot, \
    }, \
    { \
        .index = HOME_WALLET_CARD_GRAV, \
        .coin = "GRAV", \
        .chain = "Gravity Bridge", \
        .icon = &coinGrav, \
    }, \
    { \
        .index = HOME_WALLET_CARD_IXO, \
        .coin = "IXO", \
        .chain = "ixo", \
        .icon = &coinIxo, \
    }, \
    { \
        .index = HOME_WALLET_CARD_NGM, \
        .coin = "NGM", \
        .chain = "e-Money", \
        .icon = &coinNgm, \
    }, \
    { \
        .index = HOME_WALLET_CARD_IOV, \
        .coin = "IOV", \
        .chain = "Starname", \
        .icon = &coinIov, \
    }, \
    { \
        .index = HOME_WALLET_CARD_UMEE, \
        .coin = "UMEE", \
        .chain = "Umee", \
        .icon = &coinUmee, \
    }, \
    { \
        .index = HOME_WALLET_CARD_QCK, \
        .coin = "QCK", \
        .chain = "Quicksilver", \
        .icon = &coinQck, \
    }, \
    { \
        .index = HOME_WALLET_CARD_TGD, \
        .coin = "TGD", \
        .chain = "Tgrade", \
        .icon = &coinTgd, \
    }

#endif
#endif
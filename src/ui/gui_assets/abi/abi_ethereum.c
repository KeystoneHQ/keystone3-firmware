#include "abi_item.h"
#include "define.h"
#include "stdint.h"

const ABIItem_t ethereum_abi_map[] = {
    {
        "0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD_709a1cc2",
        "{\"name\":\"UniversalRouter\",\"address\":\"0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"looksRareClaim\",\"type\":\"bytes\"}],\"name\":\"collectRewards\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD_24856bc3",
        "{\"name\":\"UniversalRouter\",\"address\":\"0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"commands\",\"type\":\"bytes\"},{\"internalType\":\"bytes[]\",\"name\":\"inputs\",\"type\":\"bytes[]\"}],\"name\":\"execute\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD_3593564c",
        "{\"name\":\"UniversalRouter\",\"address\":\"0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"commands\",\"type\":\"bytes\"},{\"internalType\":\"bytes[]\",\"name\":\"inputs\",\"type\":\"bytes[]\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"execute\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD_fa461e33",
        "{\"name\":\"UniversalRouter\",\"address\":\"0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"int256\",\"name\":\"amount0Delta\",\"type\":\"int256\"},{\"internalType\":\"int256\",\"name\":\"amount1Delta\",\"type\":\"int256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"uniswapV3SwapCallback\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00000000219ab540356cbb839cbe05303d7705fa_22895118",
        "{\"name\":\"Eth2 Deposit\",\"address\":\"0x00000000219ab540356cbb839cbe05303d7705fa\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"pubkey\",\"type\":\"bytes\"},{\"internalType\":\"bytes\",\"name\":\"withdrawal_credentials\",\"type\":\"bytes\"},{\"internalType\":\"bytes\",\"name\":\"signature\",\"type\":\"bytes\"},{\"internalType\":\"bytes32\",\"name\":\"deposit_data_root\",\"type\":\"bytes32\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00d0f137b51692d0ac708bde7b367a373865cffe_25e07c3e",
        "{\"name\":\"Balancer Remove\",\"address\":\"0x00d0f137b51692D0AC708bdE7b367a373865cFfe\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_ToTokenContractAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_FromBalancerPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_IncomingBPT\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_minTokensRec\",\"type\":\"uint256\"}],\"name\":\"EasyZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00d0f137b51692d0ac708bde7b367a373865cffe_551196d5",
        "{\"name\":\"Balancer Remove\",\"address\":\"0x00d0f137b51692D0AC708bdE7b367a373865cFfe\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"_TokenAddress\",\"type\":\"address\"}],\"name\":\"inCaseTokengetsStuck\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00d0f137b51692d0ac708bde7b367a373865cffe_715018a6",
        "{\"name\":\"Balancer Remove\",\"address\":\"0x00d0f137b51692D0AC708bdE7b367a373865cFfe\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00d0f137b51692d0ac708bde7b367a373865cffe_b10e1dbc",
        "{\"name\":\"Balancer Remove\",\"address\":\"0x00d0f137b51692D0AC708bdE7b367a373865cFfe\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint16\",\"name\":\"_new_goodwill\",\"type\":\"uint16\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00d0f137b51692d0ac708bde7b367a373865cffe_1385d24c",
        "{\"name\":\"Balancer Remove\",\"address\":\"0x00d0f137b51692D0AC708bdE7b367a373865cFfe\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00d0f137b51692d0ac708bde7b367a373865cffe_f2fde38b",
        "{\"name\":\"Balancer Remove\",\"address\":\"0x00d0f137b51692D0AC708bdE7b367a373865cFfe\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x00d0f137b51692d0ac708bde7b367a373865cffe_3ccfd60b",
        "{\"name\":\"Balancer Remove\",\"address\":\"0x00d0f137b51692D0AC708bdE7b367a373865cFfe\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"withdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_095ea7b3",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_89afcb44",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"burn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amount0\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_485cc955",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_token1\",\"type\":\"address\"}],\"name\":\"initialize\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_6a627842",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"mint\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_d505accf",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"permit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_bc25cf77",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"skim\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_022c0d9f",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amount0Out\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Out\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"swap\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_fff6cae9",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"sync\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_a9059cbb",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x05767d9ef41dc40689678ffca0608878fb3de906_23b872dd",
        "{\"name\":\"CVX/ETH SLP\",\"address\":\"0x05767d9EF41dC40689678fFca0608878fb3dE906\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x111111125434b319222cdbf8c261674adb56f3ae_22102cdd",
        "{\"name\":\"1inch exchange v2\",\"address\":\"0x111111125434b319222cdbf8c261674adb56f3ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IOneInchCaller\",\"name\":\"caller\",\"type\":\"address\"},{\"components\":[{\"internalType\":\"contract IERC20\",\"name\":\"srcToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20\",\"name\":\"dstToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"srcReceiver\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"dstReceiver\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minReturnAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"guaranteedAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"flags\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"referrer\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"permit\",\"type\":\"bytes\"}],\"internalType\":\"struct OneInchExchange.SwapDescription\",\"name\":\"desc\",\"type\":\"tuple\"},{\"components\":[{\"internalType\":\"uint256\",\"name\":\"targetWithMandatory\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"gasLimit\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct IOneInchCaller.CallDescription[]\",\"name\":\"calls\",\"type\":\"tuple[]\"}],\"name\":\"discountedSwap\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"returnAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x111111125434b319222cdbf8c261674adb56f3ae_8456cb59",
        "{\"name\":\"1inch exchange v2\",\"address\":\"0x111111125434b319222cdbf8c261674adb56f3ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"pause\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x111111125434b319222cdbf8c261674adb56f3ae_715018a6",
        "{\"name\":\"1inch exchange v2\",\"address\":\"0x111111125434b319222cdbf8c261674adb56f3ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x111111125434b319222cdbf8c261674adb56f3ae_78e3214f",
        "{\"name\":\"1inch exchange v2\",\"address\":\"0x111111125434b319222cdbf8c261674adb56f3ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"rescueFunds\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x111111125434b319222cdbf8c261674adb56f3ae_32af3139",
        "{\"name\":\"1inch exchange v2\",\"address\":\"0x111111125434b319222cdbf8c261674adb56f3ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IOneInchCaller\",\"name\":\"caller\",\"type\":\"address\"},{\"components\":[{\"internalType\":\"contract IERC20\",\"name\":\"srcToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20\",\"name\":\"dstToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"srcReceiver\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"dstReceiver\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minReturnAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"guaranteedAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"flags\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"referrer\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"permit\",\"type\":\"bytes\"}],\"internalType\":\"struct OneInchExchange.SwapDescription\",\"name\":\"desc\",\"type\":\"tuple\"},{\"components\":[{\"internalType\":\"uint256\",\"name\":\"targetWithMandatory\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"gasLimit\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct IOneInchCaller.CallDescription[]\",\"name\":\"calls\",\"type\":\"tuple[]\"}],\"name\":\"swap\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"returnAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x111111125434b319222cdbf8c261674adb56f3ae_f2fde38b",
        "{\"name\":\"1inch exchange v2\",\"address\":\"0x111111125434b319222cdbf8c261674adb56f3ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x128ce9b4d97a6550905de7d9abc2b8c747b0996c_b2b63d9f",
        "{\"name\":\"WETH Pricer\",\"address\":\"0x128cE9B4D97A6550905dE7d9Abc2b8C747b0996C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_expiryTimestamp\",\"type\":\"uint256\"},{\"internalType\":\"uint80\",\"name\":\"_roundId\",\"type\":\"uint80\"}],\"name\":\"setExpiryPriceInOracle\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1389388d01708118b497f59521f6943be2541bb7_b61d27f6",
        "{\"name\":\"Treasury Vault\",\"address\":\"0x1389388d01708118b497f59521f6943Be2541bb7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"execute\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"},{\"internalType\":\"bytes\",\"name\":\"\",\"type\":\"bytes\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1389388d01708118b497f59521f6943be2541bb7_b3ab15fb",
        "{\"name\":\"Treasury Vault\",\"address\":\"0x1389388d01708118b497f59521f6943Be2541bb7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_op\",\"type\":\"address\"}],\"name\":\"setOperator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1389388d01708118b497f59521f6943be2541bb7_c4e2c1e6",
        "{\"name\":\"Treasury Vault\",\"address\":\"0x1389388d01708118b497f59521f6943Be2541bb7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_to\",\"type\":\"address\"}],\"name\":\"withdrawTo\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1766733112408b95239ad1951925567cb1203084_e343fe12",
        "{\"name\":\"SushiSwapSwapperV1\",\"address\":\"0x1766733112408b95239ad1951925567cb1203084\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"shareToMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"shareFrom\",\"type\":\"uint256\"}],\"name\":\"swap\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"extraShare\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"shareReturned\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1766733112408b95239ad1951925567cb1203084_4622be90",
        "{\"name\":\"SushiSwapSwapperV1\",\"address\":\"0x1766733112408b95239ad1951925567cb1203084\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"refundTo\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"shareFromSupplied\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"shareToExact\",\"type\":\"uint256\"}],\"name\":\"swapExact\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"shareUsed\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"shareReturned\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_715018a6",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_ca446dd9",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"_key\",\"type\":\"bytes32\"},{\"internalType\":\"address\",\"name\":\"_address\",\"type\":\"address\"}],\"name\":\"setAddress\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_92eefe9b",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_controller\",\"type\":\"address\"}],\"name\":\"setController\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_38f92fc7",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_liquidationManager\",\"type\":\"address\"}],\"name\":\"setLiquidationManager\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_e9f2e8be",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_marginCalculator\",\"type\":\"address\"}],\"name\":\"setMarginCalculator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_e7cf7841",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_marginPool\",\"type\":\"address\"}],\"name\":\"setMarginPool\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_7adbf973",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_oracle\",\"type\":\"address\"}],\"name\":\"setOracle\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_d94f323e",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_otokenFactory\",\"type\":\"address\"}],\"name\":\"setOtokenFactory\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_b508ac99",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_otokenImpl\",\"type\":\"address\"}],\"name\":\"setOtokenImpl\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_854cff2f",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_whitelist\",\"type\":\"address\"}],\"name\":\"setWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_f2fde38b",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x1e31f2dcbad4dc572004eae6355fb18f9615cbe4_2b6bfeaa",
        "{\"name\":\"AddressBook\",\"address\":\"0x1E31F2DCBad4dc572004Eae6355fB18F9615cBe4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"_id\",\"type\":\"bytes32\"},{\"internalType\":\"address\",\"name\":\"_newAddress\",\"type\":\"address\"}],\"name\":\"updateImpl\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_095ea7b3",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_spender\",\"type\":\"address\"},{\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_17ffc320",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_token\",\"type\":\"address\"}],\"name\":\"reclaimToken\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_23b872dd",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_from\",\"type\":\"address\"},{\"name\":\"_to\",\"type\":\"address\"},{\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_3f4ba83a",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"unpause\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_40c10f19",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_to\",\"type\":\"address\"},{\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"mint\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_42966c68",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"burn\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_4e71e0c8",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"claimOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_66188463",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_spender\",\"type\":\"address\"},{\"name\":\"_subtractedValue\",\"type\":\"uint256\"}],\"name\":\"decreaseApproval\",\"outputs\":[{\"name\":\"success\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_715018a6",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_7d64bcb4",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"finishMinting\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_8456cb59",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"pause\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_a9059cbb",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_to\",\"type\":\"address\"},{\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_d73dd623",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_spender\",\"type\":\"address\"},{\"name\":\"_addedValue\",\"type\":\"uint256\"}],\"name\":\"increaseApproval\",\"outputs\":[{\"name\":\"success\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2260fac5e5542a773aa44fbcfedf7c193bc2c599_f2fde38b",
        "{\"name\":\"WBTC\",\"address\":\"0x2260fac5e5542a773aa44fbcfedf7c193bc2c599\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x25e12482a25cf36ec70fda2a09c1ed077fc21616_39a0d831",
        "{\"name\":\"Arbitrator Vault\",\"address\":\"0x25E12482a25CF36EC70fDA2A09C1ED077Fc21616\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_token\",\"type\":\"address\"},{\"internalType\":\"uint256[]\",\"name\":\"_toPids\",\"type\":\"uint256[]\"},{\"internalType\":\"uint256[]\",\"name\":\"_amounts\",\"type\":\"uint256[]\"}],\"name\":\"distribute\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x25e12482a25cf36ec70fda2a09c1ed077fc21616_b3ab15fb",
        "{\"name\":\"Arbitrator Vault\",\"address\":\"0x25E12482a25CF36EC70fDA2A09C1ED077Fc21616\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_op\",\"type\":\"address\"}],\"name\":\"setOperator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_69a7e57b",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"aToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minATokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"aTokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_46d4b548",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minToTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_9462f428",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minToTokens\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"permitSig\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOutWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_0dc9de85",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_691bcc88",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"aToken\",\"type\":\"address\"}],\"name\":\"getUnderlyingToken\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_715018a6",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_9735a634",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_3ff428c7",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_fbec27bf",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_01e980d4",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_550bfa56",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_1385d24c",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_f2fde38b",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2801a71605b5e25816235c7f3cb779f4c9dd60ee_5ecb16cd",
        "{\"name\":\"Aave_Zap_V1_0_2\",\"address\":\"0x2801a71605b5E25816235C7f3Cb779F4c9dD60Ee\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_f8ba4cff",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"accrue\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_1b51e940",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"skim\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"name\":\"addAsset\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"fraction\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_860ffea1",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"skim\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"name\":\"addCollateral\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_095ea7b3",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_4b8a3529",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"borrow\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"part\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_4e71e0c8",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"claimOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_656f3d64",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint8[]\",\"name\":\"actions\",\"type\":\"uint8[]\"},{\"internalType\":\"uint256[]\",\"name\":\"values\",\"type\":\"uint256[]\"},{\"internalType\":\"bytes[]\",\"name\":\"datas\",\"type\":\"bytes[]\"}],\"name\":\"cook\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"value1\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"value2\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_4ddf47d4",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"init\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_76ee101b",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"users\",\"type\":\"address[]\"},{\"internalType\":\"uint256[]\",\"name\":\"maxBorrowParts\",\"type\":\"uint256[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"contract ISwapper\",\"name\":\"swapper\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"open\",\"type\":\"bool\"}],\"name\":\"liquidate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_d505accf",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner_\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"permit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_2317ef67",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"fraction\",\"type\":\"uint256\"}],\"name\":\"removeAsset\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_876467f8",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"name\":\"removeCollateral\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_15294c40",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"skim\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"part\",\"type\":\"uint256\"}],\"name\":\"repay\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_f46901ed",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newFeeTo\",\"type\":\"address\"}],\"name\":\"setFeeTo\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_3f2617cb",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract ISwapper\",\"name\":\"swapper\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"enable\",\"type\":\"bool\"}],\"name\":\"setSwapper\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_a9059cbb",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_23b872dd",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_078dfbe7",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"direct\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"renounce\",\"type\":\"bool\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_02ce728f",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"updateExchangeRate\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"updated\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"rate\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2cba6ab6574646badc84f0544d05059e57a5dc42_476343ee",
        "{\"name\":\"KashiPairMediumRiskV1\",\"address\":\"0x2cBA6Ab6574646Badc84F0544d05059e57a5dc42\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"withdrawFees\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2e088a0a19dda628b4304301d1ea70b114e4accd_ca21b177",
        "{\"name\":\"Airdrop\",\"address\":\"0x2E088A0A19dda628B4304301d1EA70b114e4AcCd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes32[]\",\"name\":\"_proof\",\"type\":\"bytes32[]\"},{\"internalType\":\"address\",\"name\":\"_who\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"claim\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2e088a0a19dda628b4304301d1ea70b114e4accd_4a13e91d",
        "{\"name\":\"Airdrop\",\"address\":\"0x2E088A0A19dda628B4304301d1EA70b114e4AcCd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_mintToken\",\"type\":\"address\"}],\"name\":\"setMintToken\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2e088a0a19dda628b4304301d1ea70b114e4accd_13af4035",
        "{\"name\":\"Airdrop\",\"address\":\"0x2E088A0A19dda628B4304301d1EA70b114e4AcCd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_owner\",\"type\":\"address\"}],\"name\":\"setOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2e088a0a19dda628b4304301d1ea70b114e4accd_51508f0a",
        "{\"name\":\"Airdrop\",\"address\":\"0x2E088A0A19dda628B4304301d1EA70b114e4AcCd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_rewardContract\",\"type\":\"address\"}],\"name\":\"setRewardContract\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2e088a0a19dda628b4304301d1ea70b114e4accd_8aee8127",
        "{\"name\":\"Airdrop\",\"address\":\"0x2E088A0A19dda628B4304301d1EA70b114e4AcCd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_rewardToken\",\"type\":\"address\"}],\"name\":\"setRewardToken\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2e088a0a19dda628b4304301d1ea70b114e4accd_dab5f340",
        "{\"name\":\"Airdrop\",\"address\":\"0x2E088A0A19dda628B4304301d1EA70b114e4AcCd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"_merkleRoot\",\"type\":\"bytes32\"}],\"name\":\"setRoot\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_024c7ec7",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_onlyOwnerCanUpdateRegistry\",\"type\":\"bool\"}],\"name\":\"restrictRegistryUpdate\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_02ef521e",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_token\",\"type\":\"address\"},{\"name\":\"_register\",\"type\":\"bool\"}],\"name\":\"registerEtherToken\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_2978c10e",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_beneficiary\",\"type\":\"address\"},{\"name\":\"_affiliateAccount\",\"type\":\"address\"},{\"name\":\"_affiliateFee\",\"type\":\"uint256\"}],\"name\":\"claimAndConvertFor2\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_49d10b64",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"updateRegistry\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_569706eb",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_affiliateAccount\",\"type\":\"address\"},{\"name\":\"_affiliateFee\",\"type\":\"uint256\"}],\"name\":\"convert2\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_5e35359e",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_token\",\"type\":\"address\"},{\"name\":\"_to\",\"type\":\"address\"},{\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_79ba5097",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"acceptOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_89f9cc61",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_bancorX\",\"type\":\"address\"},{\"name\":\"_conversionId\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_beneficiary\",\"type\":\"address\"}],\"name\":\"completeXConversion\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_ab6214ce",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_beneficiary\",\"type\":\"address\"},{\"name\":\"_affiliateAccount\",\"type\":\"address\"},{\"name\":\"_affiliateFee\",\"type\":\"uint256\"}],\"name\":\"convertFor2\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_b1e9932b",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_beneficiary\",\"type\":\"address\"}],\"name\":\"claimAndConvertFor\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_b4a176d3",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"restoreRegistry\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_b77d239b",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_beneficiary\",\"type\":\"address\"},{\"name\":\"_affiliateAccount\",\"type\":\"address\"},{\"name\":\"_affiliateFee\",\"type\":\"uint256\"}],\"name\":\"convertByPath\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_c52173de",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_targetBlockchain\",\"type\":\"bytes32\"},{\"name\":\"_targetAccount\",\"type\":\"bytes32\"},{\"name\":\"_conversionId\",\"type\":\"uint256\"}],\"name\":\"xConvert\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_c7ba24bc",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"}],\"name\":\"claimAndConvert\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_c98fefed",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_beneficiary\",\"type\":\"address\"}],\"name\":\"convertFor\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_cb32564e",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_targetBlockchain\",\"type\":\"bytes32\"},{\"name\":\"_targetAccount\",\"type\":\"bytes32\"},{\"name\":\"_conversionId\",\"type\":\"uint256\"},{\"name\":\"_affiliateAccount\",\"type\":\"address\"},{\"name\":\"_affiliateFee\",\"type\":\"uint256\"}],\"name\":\"xConvert2\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_e57738e5",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"},{\"name\":\"_affiliateAccount\",\"type\":\"address\"},{\"name\":\"_affiliateFee\",\"type\":\"uint256\"}],\"name\":\"claimAndConvert2\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_f2fde38b",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_f3898a97",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_path\",\"type\":\"address[]\"},{\"name\":\"_amount\",\"type\":\"uint256\"},{\"name\":\"_minReturn\",\"type\":\"uint256\"}],\"name\":\"convert\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x2f9ec37d6ccfff1cab21733bdadede11c823ccb0_f3bc7d2a",
        "{\"name\":\"Bancor\",\"address\":\"0x2F9EC37d6CcFFf1caB21733BdaDEdE11c823cCB0\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_maxAffiliateFee\",\"type\":\"uint256\"}],\"name\":\"setMaxAffiliateFee\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_247482ba",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"toTokenAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minTokensRec\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"swapTargets\",\"type\":\"address[]\"},{\"internalType\":\"bytes[]\",\"name\":\"swapData\",\"type\":\"bytes[]\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_f21d3ab5",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOut2PairToken\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_cfd7789c",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"permitSig\",\"type\":\"bytes\"}],\"name\":\"ZapOut2PairTokenWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_91027c5b",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"toTokenAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minTokensRec\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"permitSig\",\"type\":\"bytes\"},{\"internalType\":\"address[]\",\"name\":\"swapTargets\",\"type\":\"address[]\"},{\"internalType\":\"bytes[]\",\"name\":\"swapData\",\"type\":\"bytes[]\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOutWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_0dc9de85",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_715018a6",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_9735a634",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_3ff428c7",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_fbec27bf",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_01e980d4",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_550bfa56",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_1385d24c",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_f2fde38b",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3098cfaaba3795ab98dc2d5066ec4d2584ae7c68_5ecb16cd",
        "{\"name\":\"Sushiswap Remove\",\"address\":\"0x3098cFaAbA3795AB98DC2D5066eC4d2584ae7C68\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x32363cab91eeaad10bfdd0b6cd013caf2595e85d_b2b63d9f",
        "{\"name\":\"WBTC Pricer\",\"address\":\"0x32363cAB91EEaad10BFdd0b6Cd013CAF2595E85d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_expiryTimestamp\",\"type\":\"uint256\"},{\"internalType\":\"uint80\",\"name\":\"_roundId\",\"type\":\"uint80\"}],\"name\":\"setExpiryPriceInOracle\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_095ea7b3",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_89afcb44",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"burn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amount0\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_485cc955",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_token1\",\"type\":\"address\"}],\"name\":\"initialize\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_6a627842",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"mint\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_d505accf",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"permit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_bc25cf77",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"skim\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_022c0d9f",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amount0Out\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Out\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"swap\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_fff6cae9",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"sync\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_a9059cbb",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x33f6ddaea2a8a54062e021873bcaee006cdf4007_23b872dd",
        "{\"name\":\"cvxCRV/CRV SLP\",\"address\":\"0x33F6DDAEa2a8a54062E021873bCaEE006CdF4007\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x398ec7346dcd622edc5ae82352f02be94c62d119_f851a440",
        "{\"name\":\"Aave Lending pool V1\",\"address\":\"0x398ec7346dcd622edc5ae82352f02be94c62d119\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"admin\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x398ec7346dcd622edc5ae82352f02be94c62d119_8f283970",
        "{\"name\":\"Aave Lending pool V1\",\"address\":\"0x398ec7346dcd622edc5ae82352f02be94c62d119\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newAdmin\",\"type\":\"address\"}],\"name\":\"changeAdmin\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x398ec7346dcd622edc5ae82352f02be94c62d119_5c60da1b",
        "{\"name\":\"Aave Lending pool V1\",\"address\":\"0x398ec7346dcd622edc5ae82352f02be94c62d119\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"implementation\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x398ec7346dcd622edc5ae82352f02be94c62d119_cf7a1d77",
        "{\"name\":\"Aave Lending pool V1\",\"address\":\"0x398ec7346dcd622edc5ae82352f02be94c62d119\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_logic\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_admin\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"initialize\",\"outputs\":[],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x398ec7346dcd622edc5ae82352f02be94c62d119_d1f57894",
        "{\"name\":\"Aave Lending pool V1\",\"address\":\"0x398ec7346dcd622edc5ae82352f02be94c62d119\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_logic\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"initialize\",\"outputs\":[],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x398ec7346dcd622edc5ae82352f02be94c62d119_3659cfe6",
        "{\"name\":\"Aave Lending pool V1\",\"address\":\"0x398ec7346dcd622edc5ae82352f02be94c62d119\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newImplementation\",\"type\":\"address\"}],\"name\":\"upgradeTo\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x398ec7346dcd622edc5ae82352f02be94c62d119_4f1ef286",
        "{\"name\":\"Aave Lending pool V1\",\"address\":\"0x398ec7346dcd622edc5ae82352f02be94c62d119\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newImplementation\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"upgradeToAndCall\",\"outputs\":[],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3b2d30734491ad77880d31e8afe2a4d8ac135a9c_7e29d6c2",
        "{\"name\":\"Pool Manager\",\"address\":\"0x3b2D30734491AD77880d31e8aFe2A4d8aC135a9C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_swap\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_stashVersion\",\"type\":\"uint256\"}],\"name\":\"addPool\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3b2d30734491ad77880d31e8afe2a4d8ac135a9c_f7b48e96",
        "{\"name\":\"Pool Manager\",\"address\":\"0x3b2D30734491AD77880d31e8aFe2A4d8aC135a9C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"revertControl\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3b2d30734491ad77880d31e8afe2a4d8ac135a9c_b3ab15fb",
        "{\"name\":\"Pool Manager\",\"address\":\"0x3b2D30734491AD77880d31e8aFe2A4d8aC135a9C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_operator\",\"type\":\"address\"}],\"name\":\"setOperator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3b2d30734491ad77880d31e8afe2a4d8ac135a9c_60cafe84",
        "{\"name\":\"Pool Manager\",\"address\":\"0x3b2D30734491AD77880d31e8aFe2A4d8aC135a9C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"shutdownPool\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3c0638bb4b2bec6d89c09ab4a7f9e21e4586189b_9c23da50",
        "{\"name\":\"Permit Callee\",\"address\":\"0x3c0638bb4b2bec6d89c09ab4a7f9e21e4586189b\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"_sender\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"callFunction\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3c995e43e6ddd551e226f4c5544c77bfed147ab9_114a899c",
        "{\"name\":\"Token Factory\",\"address\":\"0x3c995e43E6ddD551E226F4c5544C77BfeD147aB9\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_lptoken\",\"type\":\"address\"}],\"name\":\"CreateDepositToken\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_0d582f13",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"addOwnerWithThreshold\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_d4d9bdcd",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"hashToApprove\",\"type\":\"bytes32\"}],\"name\":\"approveHash\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_694e80c3",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"changeThreshold\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_e009cfde",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevModule\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"disableModule\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_610b5925",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"enableModule\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_6a761202",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"},{\"internalType\":\"uint256\",\"name\":\"safeTxGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"baseGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"gasPrice\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"gasToken\",\"type\":\"address\"},{\"internalType\":\"address payable\",\"name\":\"refundReceiver\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"signatures\",\"type\":\"bytes\"}],\"name\":\"execTransaction\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_468721a7",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"execTransactionFromModule\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_5229073f",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"execTransactionFromModuleReturnData\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"},{\"internalType\":\"bytes\",\"name\":\"returnData\",\"type\":\"bytes\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_f8dc5dd9",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"removeOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_c4ca3a9c",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"requiredTxGas\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_f08a0323",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"handler\",\"type\":\"address\"}],\"name\":\"setFallbackHandler\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_e19a9dd9",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"guard\",\"type\":\"address\"}],\"name\":\"setGuard\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_b63e800d",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"_owners\",\"type\":\"address[]\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"fallbackHandler\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"paymentToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"payment\",\"type\":\"uint256\"},{\"internalType\":\"address payable\",\"name\":\"paymentReceiver\",\"type\":\"address\"}],\"name\":\"setup\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_b4faba09",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"targetContract\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"calldataPayload\",\"type\":\"bytes\"}],\"name\":\"simulateAndRevert\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3e5c63644e683549055b9be8653de26e0b4cd36e_e318b52b",
        "{\"name\":\"Gnosis Safe: Singleton L2 1.3.0\",\"address\":\"0x3e5c63644e683549055b9be8653de26e0b4cd36e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"oldOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"swapOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_24021127",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"originationFeeMantissa\",\"type\":\"uint256\"}],\"name\":\"_setOriginationFee\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_26617c28",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"requestedState\",\"type\":\"bool\"}],\"name\":\"_setPaused\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_3be59443",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newOracle\",\"type\":\"address\"}],\"name\":\"_setOracle\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_4706c375",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"},{\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"_withdrawEquity\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_4b8a3529",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"},{\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"borrow\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_5cf756d2",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"},{\"name\":\"interestRateModel\",\"type\":\"address\"}],\"name\":\"_setMarketInterestRateModel\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_abdb5ea8",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"},{\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"repayBorrow\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_b71d1a0c",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newPendingAdmin\",\"type\":\"address\"}],\"name\":\"_setPendingAdmin\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_c1abfaa3",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"},{\"name\":\"interestRateModel\",\"type\":\"address\"}],\"name\":\"_supportMarket\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_c365a646",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"collateralRatioMantissa\",\"type\":\"uint256\"},{\"name\":\"liquidationDiscountMantissa\",\"type\":\"uint256\"}],\"name\":\"_setRiskParameters\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_dbe2bc84",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"}],\"name\":\"_suspendMarket\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_e61604cf",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"targetAccount\",\"type\":\"address\"},{\"name\":\"assetBorrow\",\"type\":\"address\"},{\"name\":\"assetCollateral\",\"type\":\"address\"},{\"name\":\"requestedAmountClose\",\"type\":\"uint256\"}],\"name\":\"liquidateBorrow\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_e9c714f2",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"_acceptAdmin\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_f2b9fdb8",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"},{\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"supply\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fda67f7583380e67ef93072294a7fac882fd7e7_f3fef3a3",
        "{\"name\":\"Compound\",\"address\":\"0x3fda67f7583380e67ef93072294a7fac882fd7e7\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"asset\",\"type\":\"address\"},{\"name\":\"requestedAmount\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_5e43c47b",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_reward\",\"type\":\"address\"}],\"name\":\"addExtraReward\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_0569d388",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"clearExtraRewards\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_f14faf6f",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"donate\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_3d18b912",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"getReward\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_7050ccd9",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_account\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_claimExtras\",\"type\":\"bool\"}],\"name\":\"getReward\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_590a41f5",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_rewards\",\"type\":\"uint256\"}],\"name\":\"queueNewRewards\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_a694fc3a",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"stake\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_8dcb4061",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"stakeAll\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_2ee40908",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_for\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"stakeFor\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_38d07436",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"claim\",\"type\":\"bool\"}],\"name\":\"withdraw\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_1c1c6fe5",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bool\",\"name\":\"claim\",\"type\":\"bool\"}],\"name\":\"withdrawAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_49f039a2",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bool\",\"name\":\"claim\",\"type\":\"bool\"}],\"name\":\"withdrawAllAndUnwrap\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x3fe65692bfcd0e6cf84cb1e7d24108e434a7587e_c32e7202",
        "{\"name\":\"cvxCRV Rewards\",\"address\":\"0x3Fe65692bfCD0e6CF84cB1E7d24108E434A7587e\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"claim\",\"type\":\"bool\"}],\"name\":\"withdrawAndUnwrap\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_247482ba",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"toTokenAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minTokensRec\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"swapTargets\",\"type\":\"address[]\"},{\"internalType\":\"bytes[]\",\"name\":\"swapData\",\"type\":\"bytes[]\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_f21d3ab5",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOut2PairToken\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_cfd7789c",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"permitSig\",\"type\":\"bytes\"}],\"name\":\"ZapOut2PairTokenWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_91027c5b",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"toTokenAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"fromPoolAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingLP\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minTokensRec\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"permitSig\",\"type\":\"bytes\"},{\"internalType\":\"address[]\",\"name\":\"swapTargets\",\"type\":\"address[]\"},{\"internalType\":\"bytes[]\",\"name\":\"swapData\",\"type\":\"bytes[]\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOutWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_0dc9de85",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_715018a6",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_9735a634",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_3ff428c7",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_fbec27bf",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_01e980d4",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_550bfa56",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_1385d24c",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_f2fde38b",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4422610f73f4143a3936f8fa300329bba8833b54_5ecb16cd",
        "{\"name\":\"Uniswap V2 Remove\",\"address\":\"0x4422610F73f4143a3936F8fa300329BBa8833b54\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_049878f3",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"join\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_13af4035",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"owner_\",\"type\":\"address\"}],\"name\":\"setOwner\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_338a0261",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"rhi\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_343aad82",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"flow\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_40cc8854",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"}],\"name\":\"bite\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_440f19ba",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"},{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"draw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_6f78ee0d",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"}],\"name\":\"rap\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_73b38101",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"},{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"wipe\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_7a9e5e4b",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"authority_\",\"type\":\"address\"}],\"name\":\"setAuthority\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_7e74325f",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"tap_\",\"type\":\"address\"}],\"name\":\"turn\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_7f8661a1",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"exit\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_82bf9a75",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"pip_\",\"type\":\"address\"}],\"name\":\"setPip\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_8ceedb47",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"fit_\",\"type\":\"uint256\"},{\"name\":\"jam\",\"type\":\"uint256\"}],\"name\":\"cage\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_92b0d721",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"param\",\"type\":\"bytes32\"},{\"name\":\"val\",\"type\":\"uint256\"}],\"name\":\"mold\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_9f678cca",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"drip\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_a5cd184e",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"},{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"free\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_b3b77a51",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"},{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"lock\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_b84d2106",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"}],\"name\":\"shut\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_baa8529c",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"},{\"name\":\"guy\",\"type\":\"address\"}],\"name\":\"give\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_c92aecc4",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"chi\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_cf48d1a6",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"vox_\",\"type\":\"address\"}],\"name\":\"setVox\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_d9c27cc6",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"pep_\",\"type\":\"address\"}],\"name\":\"setPep\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_e0ae96e9",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"din\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_e95823ad",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"}],\"name\":\"safe\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_f7c8d634",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"}],\"name\":\"tab\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x448a5065aebb8e423f0896e6c5d525c040f59af3_fcfff16f",
        "{\"name\":\"Maker\",\"address\":\"0x448a5065aebb8e423f0896e6c5d525c040f59af3\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"open\",\"outputs\":[{\"name\":\"cup\",\"type\":\"bytes32\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_49ee0fa6",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"prizePool\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minTickets\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapIn\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_0dc9de85",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_715018a6",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_9735a634",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_3ff428c7",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_fbec27bf",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_01e980d4",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_550bfa56",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_1385d24c",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_f2fde38b",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4752b3bf9dabd61fe8150760ec580b183d9fda57_5ecb16cd",
        "{\"name\":\"PoolTogether Add\",\"address\":\"0x4752b3Bf9DAbD61FE8150760EC580b183D9fdA57\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4ccc2339f87f6c59c6893e1a678c2266ca58dc72_f1739cae",
        "{\"name\":\"Controller proxy\",\"address\":\"0x4ccc2339F87F6c59c6893E1A678c2266cA58dC72\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_newOwner\",\"type\":\"address\"}],\"name\":\"transferProxyOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4ccc2339f87f6c59c6893e1a678c2266ca58dc72_3659cfe6",
        "{\"name\":\"Controller proxy\",\"address\":\"0x4ccc2339F87F6c59c6893E1A678c2266cA58dC72\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_implementation\",\"type\":\"address\"}],\"name\":\"upgradeTo\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4ccc2339f87f6c59c6893e1a678c2266ca58dc72_4f1ef286",
        "{\"name\":\"Controller proxy\",\"address\":\"0x4ccc2339F87F6c59c6893E1A678c2266cA58dC72\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_implementation\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"upgradeToAndCall\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b_095ea7b3",
        "{\"name\":\"CVX\",\"address\":\"0x4e3FBD56CD56c3e72c1403e103b45Db9da5B9D2B\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b_a457c2d7",
        "{\"name\":\"CVX\",\"address\":\"0x4e3FBD56CD56c3e72c1403e103b45Db9da5B9D2B\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"subtractedValue\",\"type\":\"uint256\"}],\"name\":\"decreaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b_39509351",
        "{\"name\":\"CVX\",\"address\":\"0x4e3FBD56CD56c3e72c1403e103b45Db9da5B9D2B\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"addedValue\",\"type\":\"uint256\"}],\"name\":\"increaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b_40c10f19",
        "{\"name\":\"CVX\",\"address\":\"0x4e3FBD56CD56c3e72c1403e103b45Db9da5B9D2B\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"mint\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b_a9059cbb",
        "{\"name\":\"CVX\",\"address\":\"0x4e3FBD56CD56c3e72c1403e103b45Db9da5B9D2B\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b_23b872dd",
        "{\"name\":\"CVX\",\"address\":\"0x4e3FBD56CD56c3e72c1403e103b45Db9da5B9D2B\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"sender\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e3fbd56cd56c3e72c1403e103b45db9da5b9d2b_d5934b76",
        "{\"name\":\"CVX\",\"address\":\"0x4e3FBD56CD56c3e72c1403e103b45Db9da5B9D2B\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"updateOperator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_ab033ea9",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"setGovernance\",\"inputs\":[{\"name\":\"governance\",\"type\":\"address\"}],\"outputs\":[],\"gas\":36245}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_238efcbc",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"acceptGovernance\",\"inputs\":[],\"outputs\":[],\"gas\":37517}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_33990d4b",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"newRelease\",\"inputs\":[{\"name\":\"vault\",\"type\":\"address\"}],\"outputs\":[],\"gas\":82588}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_108ca11e",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"newVault\",\"inputs\":[{\"name\":\"token\",\"type\":\"address\"},{\"name\":\"guardian\",\"type\":\"address\"},{\"name\":\"rewards\",\"type\":\"address\"},{\"name\":\"name\",\"type\":\"string\"},{\"name\":\"symbol\",\"type\":\"string\"}],\"outputs\":[{\"name\":\"\",\"type\":\"address\"}]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_b0b40fce",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"newVault\",\"inputs\":[{\"name\":\"token\",\"type\":\"address\"},{\"name\":\"guardian\",\"type\":\"address\"},{\"name\":\"rewards\",\"type\":\"address\"},{\"name\":\"name\",\"type\":\"string\"},{\"name\":\"symbol\",\"type\":\"string\"},{\"name\":\"releaseDelta\",\"type\":\"uint256\"}],\"outputs\":[{\"name\":\"\",\"type\":\"address\"}]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_5b73aa0d",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"newExperimentalVault\",\"inputs\":[{\"name\":\"token\",\"type\":\"address\"},{\"name\":\"governance\",\"type\":\"address\"},{\"name\":\"guardian\",\"type\":\"address\"},{\"name\":\"rewards\",\"type\":\"address\"},{\"name\":\"name\",\"type\":\"string\"},{\"name\":\"symbol\",\"type\":\"string\"}],\"outputs\":[{\"name\":\"\",\"type\":\"address\"}]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_5bd4b0f2",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"newExperimentalVault\",\"inputs\":[{\"name\":\"token\",\"type\":\"address\"},{\"name\":\"governance\",\"type\":\"address\"},{\"name\":\"guardian\",\"type\":\"address\"},{\"name\":\"rewards\",\"type\":\"address\"},{\"name\":\"name\",\"type\":\"string\"},{\"name\":\"symbol\",\"type\":\"string\"},{\"name\":\"releaseDelta\",\"type\":\"uint256\"}],\"outputs\":[{\"name\":\"\",\"type\":\"address\"}]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_29b2e0c6",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"endorseVault\",\"inputs\":[{\"name\":\"vault\",\"type\":\"address\"}],\"outputs\":[]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_b366a35c",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"endorseVault\",\"inputs\":[{\"name\":\"vault\",\"type\":\"address\"},{\"name\":\"releaseDelta\",\"type\":\"uint256\"}],\"outputs\":[]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_2cad8f9f",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"setBanksy\",\"inputs\":[{\"name\":\"tagger\",\"type\":\"address\"}],\"outputs\":[]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_5e05f6af",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"setBanksy\",\"inputs\":[{\"name\":\"tagger\",\"type\":\"address\"},{\"name\":\"allowed\",\"type\":\"bool\"}],\"outputs\":[]}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x50c1a2ea0a861a967d9d0ffe2ae4012c2e053804_60bd68f8",
        "{\"name\":\"Yearn.finance\",\"address\":\"0x50c1a2eA0a861A967D9d0FFE2AE4012c2E053804\",\"metadata\":{\"output\":{\"abi\":[{\"stateMutability\":\"nonpayable\",\"type\":\"function\",\"name\":\"tagVault\",\"inputs\":[{\"name\":\"vault\",\"type\":\"address\"},{\"name\":\"tag\",\"type\":\"string\"}],\"outputs\":[],\"gas\":186064}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_ddcafb99",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"toPool\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minPoolTokens\",\"type\":\"uint256\"},{\"internalType\":\"uint256[]\",\"name\":\"fromTokenAmounts\",\"type\":\"uint256[]\"},{\"internalType\":\"address[]\",\"name\":\"swapTargets\",\"type\":\"address[]\"},{\"internalType\":\"bytes[]\",\"name\":\"swapData\",\"type\":\"bytes[]\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"lpReceived\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_0dc9de85",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_715018a6",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_3ff428c7",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_fbec27bf",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_01e980d4",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_550bfa56",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_1385d24c",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_f2fde38b",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5531b2eaa05d8b6fb855e15f1d21e6a3a3794b4d_5ecb16cd",
        "{\"name\":\"1Inch Add\",\"address\":\"0x5531b2eAA05D8b6fb855E15F1d21e6a3A3794B4d\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_4979cd14",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"_asset\",\"type\":\"address[]\"},{\"internalType\":\"address[]\",\"name\":\"_user\",\"type\":\"address[]\"},{\"internalType\":\"uint256[]\",\"name\":\"_amount\",\"type\":\"uint256[]\"}],\"name\":\"batchTransferToPool\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_86a19c5e",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"_asset\",\"type\":\"address[]\"},{\"internalType\":\"address[]\",\"name\":\"_user\",\"type\":\"address[]\"},{\"internalType\":\"uint256[]\",\"name\":\"_amount\",\"type\":\"uint256[]\"}],\"name\":\"batchTransferToUser\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_baf46ba6",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_receiver\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"farm\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_715018a6",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_e2ed781c",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_farmer\",\"type\":\"address\"}],\"name\":\"setFarmer\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_f2fde38b",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_dd2c99f7",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_user\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"transferToPool\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5934807cc0654d46755ebd2848840b616256c6ef_fa93b2a5",
        "{\"name\":\"MarginPool\",\"address\":\"0x5934807cC0654d46755eBd2848840b616256C6Ef\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_user\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"transferToUser\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_9ca37a0b",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_FromTokenContractAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_pairAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_minPoolTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"transferResidual\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_0dc9de85",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_715018a6",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_9735a634",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_3ff428c7",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_fbec27bf",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_01e980d4",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_550bfa56",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_1385d24c",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_f2fde38b",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5abfbe56553a5d794330eaccf556ca1d2a55647c_5ecb16cd",
        "{\"name\":\"Sushiswap Add\",\"address\":\"0x5abfbE56553a5d794330EACCF556Ca1d2a55647C\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_c71b4b43",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_allocPoint\",\"type\":\"uint256\"},{\"internalType\":\"contract IERC20\",\"name\":\"_lpToken\",\"type\":\"address\"},{\"internalType\":\"contract IRewarder\",\"name\":\"_rewarder\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_withUpdate\",\"type\":\"bool\"}],\"name\":\"add\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_ddd5e1b2",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_account\",\"type\":\"address\"}],\"name\":\"claim\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_e2bbb158",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_5312ea8e",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"emergencyWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_630b5ba1",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"massUpdatePools\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_715018a6",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_3deb2c10",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_allocPoint\",\"type\":\"uint256\"},{\"internalType\":\"contract IRewarder\",\"name\":\"_rewarder\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_withUpdate\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"_updateRewarder\",\"type\":\"bool\"}],\"name\":\"set\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_f2fde38b",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_51eb05a6",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"updatePool\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x5f465e9fcffc217c5849906216581a657cd60605_441a3e70",
        "{\"name\":\"Convex MasterChef\",\"address\":\"0x5F465e9fcfFc217c5849906216581a657cd60605\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_095ea7b3",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_9dc29fac",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_from\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"burn\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_a457c2d7",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"subtractedValue\",\"type\":\"uint256\"}],\"name\":\"decreaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_39509351",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"addedValue\",\"type\":\"uint256\"}],\"name\":\"increaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_40c10f19",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"mint\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_b3ab15fb",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_operator\",\"type\":\"address\"}],\"name\":\"setOperator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_a9059cbb",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x62b9c7356a2dc64a1969e19c23e4f579f9810aa7_23b872dd",
        "{\"name\":\"cvxCRV\",\"address\":\"0x62B9c7356A2Dc64a1969e19C23e4f579F9810Aa7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"sender\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_571ac8b0",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"}],\"name\":\"approveMax\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_cab372ce",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"}],\"name\":\"approveMaxMinusOne\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_639d71a9",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"}],\"name\":\"approveZeroThenMax\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_ab3fdd50",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"}],\"name\":\"approveZeroThenMaxMinusOne\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_b3a2af13",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"callPositionManager\",\"outputs\":[{\"internalType\":\"bytes\",\"name\":\"result\",\"type\":\"bytes\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_80fb3ad6",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"bytes\",\"name\":\"path\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMinimum\",\"type\":\"uint256\"}],\"internalType\":\"struct IV3SwapRouter.ExactInputParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactInput\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_5d76b977",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"tokenIn\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenOut\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMinimum\",\"type\":\"uint256\"},{\"internalType\":\"uint160\",\"name\":\"sqrtPriceLimitX96\",\"type\":\"uint160\"}],\"internalType\":\"struct IV3SwapRouter.ExactInputSingleParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactInputSingle\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_d42bbb58",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"bytes\",\"name\":\"path\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMaximum\",\"type\":\"uint256\"}],\"internalType\":\"struct IV3SwapRouter.ExactOutputParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactOutput\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_5bd7800f",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"tokenIn\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenOut\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMaximum\",\"type\":\"uint256\"},{\"internalType\":\"uint160\",\"name\":\"sqrtPriceLimitX96\",\"type\":\"uint160\"}],\"internalType\":\"struct IV3SwapRouter.ExactOutputSingleParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactOutputSingle\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_dee00f35",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"getApprovalType\",\"outputs\":[{\"internalType\":\"enum IApproveAndCall.ApprovalType\",\"name\":\"\",\"type\":\"uint8\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_f13884c1",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"token1\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount0Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Min\",\"type\":\"uint256\"}],\"internalType\":\"struct IApproveAndCall.IncreaseLiquidityParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"increaseLiquidity\",\"outputs\":[{\"internalType\":\"bytes\",\"name\":\"result\",\"type\":\"bytes\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_4405fca9",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"token1\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"int24\",\"name\":\"tickLower\",\"type\":\"int24\"},{\"internalType\":\"int24\",\"name\":\"tickUpper\",\"type\":\"int24\"},{\"internalType\":\"uint256\",\"name\":\"amount0Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Min\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"internalType\":\"struct IApproveAndCall.MintParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"mint\",\"outputs\":[{\"internalType\":\"bytes\",\"name\":\"result\",\"type\":\"bytes\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_1f0464d1",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"previousBlockhash\",\"type\":\"bytes32\"},{\"internalType\":\"bytes[]\",\"name\":\"data\",\"type\":\"bytes[]\"}],\"name\":\"multicall\",\"outputs\":[{\"internalType\":\"bytes[]\",\"name\":\"\",\"type\":\"bytes[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_5ae401dc",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bytes[]\",\"name\":\"data\",\"type\":\"bytes[]\"}],\"name\":\"multicall\",\"outputs\":[{\"internalType\":\"bytes[]\",\"name\":\"\",\"type\":\"bytes[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_ac9650d8",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes[]\",\"name\":\"data\",\"type\":\"bytes[]\"}],\"name\":\"multicall\",\"outputs\":[{\"internalType\":\"bytes[]\",\"name\":\"results\",\"type\":\"bytes[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_f2d5d56b",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"pull\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_12210e8a",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"refundETH\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_f3995c67",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_4659a494",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowed\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_a4a78f0c",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowedIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_c2e3140a",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_472b43f3",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"swapExactTokensForTokens\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_42712a67",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMax\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"}],\"name\":\"swapTokensForExactTokens\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_df2ab5bb",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"sweepToken\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_e90a182f",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"}],\"name\":\"sweepToken\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_3068c554",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"feeBips\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"feeRecipient\",\"type\":\"address\"}],\"name\":\"sweepTokenWithFee\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_e0e189a0",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"feeBips\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"feeRecipient\",\"type\":\"address\"}],\"name\":\"sweepTokenWithFee\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_fa461e33",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"int256\",\"name\":\"amount0Delta\",\"type\":\"int256\"},{\"internalType\":\"int256\",\"name\":\"amount1Delta\",\"type\":\"int256\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"uniswapV3SwapCallback\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_49404b7c",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"unwrapWETH9\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_49616997",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"}],\"name\":\"unwrapWETH9\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_9b2c0a37",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"feeBips\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"feeRecipient\",\"type\":\"address\"}],\"name\":\"unwrapWETH9WithFee\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_d4ef38de",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"feeBips\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"feeRecipient\",\"type\":\"address\"}],\"name\":\"unwrapWETH9WithFee\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x68b3465833fb72a70ecdf485e0e4c7bd8665fc45_1c58db4f",
        "{\"name\":\"SwapRouter02\",\"address\":\"0x68b3465833fb72A70ecDF485E0e4C7bD8665Fc45\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"wrapETH\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_9ca37a0b",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_FromTokenContractAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_pairAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_minPoolTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"transferResidual\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_0dc9de85",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_715018a6",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_9735a634",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_3ff428c7",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_fbec27bf",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_01e980d4",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_550bfa56",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_1385d24c",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_f2fde38b",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x6d9893fa101cd2b1f8d1a12de3189ff7b80fdc10_5ecb16cd",
        "{\"name\":\"Uniswap V2 Add\",\"address\":\"0x6D9893fa101CD2b1F8D1A12DE3189ff7b80FdC10\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_57a37dca",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_expiryTimestamp\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_price\",\"type\":\"uint256\"}],\"name\":\"disputeExpiryPrice\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_6c525d04",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"endMigration\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_2e4cf6ea",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"uint256[]\",\"name\":\"_expiries\",\"type\":\"uint256[]\"},{\"internalType\":\"uint256[]\",\"name\":\"_prices\",\"type\":\"uint256[]\"}],\"name\":\"migrateOracle\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_715018a6",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_3fd3ec8f",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_pricer\",\"type\":\"address\"}],\"name\":\"setAssetPricer\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_8b3cddaf",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_pricer\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_disputePeriod\",\"type\":\"uint256\"}],\"name\":\"setDisputePeriod\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_8ee5074d",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_disputer\",\"type\":\"address\"}],\"name\":\"setDisputer\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_ee531409",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_expiryTimestamp\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_price\",\"type\":\"uint256\"}],\"name\":\"setExpiryPrice\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_f19ae734",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_pricer\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_lockingPeriod\",\"type\":\"uint256\"}],\"name\":\"setLockingPeriod\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_601407eb",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_price\",\"type\":\"uint256\"}],\"name\":\"setStablePrice\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x789cd7ab3742e23ce0952f6bc3eb3a73a0e08833_f2fde38b",
        "{\"name\":\"Oracle\",\"address\":\"0x789cD7AB3742e23Ce0952F6Bc3Eb3A73A0E08833\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_49ee0fa6",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromTokenAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"toTokenAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"swapAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingTokenQty\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minPoolTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"crvTokensBought\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_0dc9de85",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_715018a6",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_9735a634",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_3ff428c7",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_fbec27bf",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_01e980d4",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_550bfa56",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_1385d24c",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_f2fde38b",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_e5953382",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract ICurveRegistry\",\"name\":\"newCurveRegistry\",\"type\":\"address\"}],\"name\":\"updateCurveRegistry\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a0a6906de7714d27413f5092ed1a0636a3fbc9a_5ecb16cd",
        "{\"name\":\"Curve Add\",\"address\":\"0x7A0a6906De7714d27413f5092ED1a0636A3FBc9A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_e8e33700",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"tokenA\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenB\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountADesired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBDesired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountAMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"addLiquidity\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_f305d719",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenDesired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"addLiquidityETH\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountToken\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_baa2abde",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"tokenA\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenB\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountAMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"removeLiquidity\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_02751cec",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"removeLiquidityETH\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountToken\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_af2979eb",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"removeLiquidityETHSupportingFeeOnTransferTokens\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_ded9382a",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"approveMax\",\"type\":\"bool\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"removeLiquidityETHWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountToken\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_5b0d5984",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"approveMax\",\"type\":\"bool\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"removeLiquidityETHWithPermitSupportingFeeOnTransferTokens\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_2195995c",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"tokenA\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenB\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountAMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"approveMax\",\"type\":\"bool\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"removeLiquidityWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_fb3bdb41",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapETHForExactTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_7ff36ab5",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactETHForTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_b6f9de95",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactETHForTokensSupportingFeeOnTransferTokens\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_18cbafe5",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForETH\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_791ac947",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForETHSupportingFeeOnTransferTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_38ed1739",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_5c11d795",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForTokensSupportingFeeOnTransferTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_4a25d94a",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMax\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapTokensForExactETH\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7a250d5630b4cf539739df2c5dacb4c659f2488d_8803dbee",
        "{\"name\":\"Uniswap V2\",\"address\":\"0x7a250d5630b4cf539739df2c5dacb4c659f2488d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMax\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapTokensForExactTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c06792af1632e77cb27a558dc0885338f4bdf8e_c0974630",
        "{\"name\":\"OtokenFactory\",\"address\":\"0x7C06792Af1632E77cb27a558Dc0885338F4Bdf8E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_underlyingAsset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_strikeAsset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_collateralAsset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_strikePrice\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_expiry\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"_isPut\",\"type\":\"bool\"}],\"name\":\"createOtoken\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_095ea7b3",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_56d878f7",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"account\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"burnOtoken\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_a457c2d7",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"subtractedValue\",\"type\":\"uint256\"}],\"name\":\"decreaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_39509351",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"addedValue\",\"type\":\"uint256\"}],\"name\":\"increaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_f630df34",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_addressBook\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_underlyingAsset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_strikeAsset\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_collateralAsset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_strikePrice\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_expiryTimestamp\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"_isPut\",\"type\":\"bool\"}],\"name\":\"init\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_51b0a410",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"account\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"mintOtoken\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_d505accf",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"permit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_a9059cbb",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7c91794b65eb573c3702229009acd3cde712146d_23b872dd",
        "{\"name\":\"Otoken\",\"address\":\"0x7C91794b65eB573c3702229009AcD3CDe712146D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"sender\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_0e29df22",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"sendEther\",\"outputs\":[],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_0ea9c984",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"changeDependentContractAddress\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_2d9fa500",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"curr\",\"type\":\"bytes4\"}],\"name\":\"internalLiquiditySwap\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_4a0d95d6",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"curr\",\"type\":\"bytes4[]\"},{\"name\":\"rate\",\"type\":\"uint64[]\"},{\"name\":\"date\",\"type\":\"uint64\"},{\"name\":\"bit\",\"type\":\"bool\"}],\"name\":\"saveIADetails\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_6dce9b3d",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newPoolAddress\",\"type\":\"address\"}],\"name\":\"upgradeInvestmentPool\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_b19ab66d",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newFactoryAddress\",\"type\":\"address\"}],\"name\":\"changeUniswapFactoryAddress\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_d46655f4",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_masterAddress\",\"type\":\"address\"}],\"name\":\"changeMasterAddress\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7cbe5682be6b648cc1100c76d4f6c96997f753d6_f720036c",
        "{\"name\":\"Nexus Mutual\",\"address\":\"0x7cbe5682be6b648cc1100c76d4f6c96997f753d6\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"externalLiquidityTrade\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9_f851a440",
        "{\"name\":\"Aave Lending pool V2\",\"address\":\"0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"admin\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9_5c60da1b",
        "{\"name\":\"Aave Lending pool V2\",\"address\":\"0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"implementation\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9_d1f57894",
        "{\"name\":\"Aave Lending pool V2\",\"address\":\"0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_logic\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"initialize\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9_3659cfe6",
        "{\"name\":\"Aave Lending pool V2\",\"address\":\"0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newImplementation\",\"type\":\"address\"}],\"name\":\"upgradeTo\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9_4f1ef286",
        "{\"name\":\"Aave Lending pool V2\",\"address\":\"0x7d2768de32b0b80b7a3454c06bdac94a69ddc7a9\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newImplementation\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"upgradeToAndCall\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_8286e89a",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_FromTokenContractAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_ToBalancerPoolAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_toTokenContractAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_minPoolTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_allowanceTarget\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapCallData\",\"type\":\"bytes\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"LPTRec\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_551196d5",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"_TokenAddress\",\"type\":\"address\"}],\"name\":\"inCaseTokengetsStuck\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_715018a6",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_b10e1dbc",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint16\",\"name\":\"_new_goodwill\",\"type\":\"uint16\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_7810bf19",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"_new_zgoodwillAddress\",\"type\":\"address\"}],\"name\":\"set_new_zgoodwillAddress\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_1385d24c",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_f2fde38b",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x7f249dbd212158e1ac449f0a37ca956c8186ac80_3ccfd60b",
        "{\"name\":\"Balancer Add\",\"address\":\"0x7F249DBD212158e1ac449f0A37CA956C8186ac80\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"withdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8014595f2ab54cd7c604b00e9fb932176fdc86ae_80ed71e4",
        "{\"name\":\"CRV Depositor\",\"address\":\"0x8014595F2AB54cD7c604B00E9fb932176fDc86Ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"_lock\",\"type\":\"bool\"},{\"internalType\":\"address\",\"name\":\"_stakeAddress\",\"type\":\"address\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8014595f2ab54cd7c604b00e9fb932176fdc86ae_9a408321",
        "{\"name\":\"CRV Depositor\",\"address\":\"0x8014595F2AB54cD7c604B00E9fb932176fDc86Ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"_lock\",\"type\":\"bool\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8014595f2ab54cd7c604b00e9fb932176fdc86ae_215537fd",
        "{\"name\":\"CRV Depositor\",\"address\":\"0x8014595F2AB54cD7c604B00E9fb932176fDc86Ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bool\",\"name\":\"_lock\",\"type\":\"bool\"},{\"internalType\":\"address\",\"name\":\"_stakeAddress\",\"type\":\"address\"}],\"name\":\"depositAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8014595f2ab54cd7c604b00e9fb932176fdc86ae_836f8f20",
        "{\"name\":\"CRV Depositor\",\"address\":\"0x8014595F2AB54cD7c604B00E9fb932176fDc86Ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"initialLock\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8014595f2ab54cd7c604b00e9fb932176fdc86ae_1caf4b2f",
        "{\"name\":\"CRV Depositor\",\"address\":\"0x8014595F2AB54cD7c604B00E9fb932176fDc86Ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"lockCurve\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8014595f2ab54cd7c604b00e9fb932176fdc86ae_472d35b9",
        "{\"name\":\"CRV Depositor\",\"address\":\"0x8014595F2AB54cD7c604B00E9fb932176fDc86Ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_feeManager\",\"type\":\"address\"}],\"name\":\"setFeeManager\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8014595f2ab54cd7c604b00e9fb932176fdc86ae_3d18678e",
        "{\"name\":\"CRV Depositor\",\"address\":\"0x8014595F2AB54cD7c604B00E9fb932176fDc86Ae\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_lockIncentive\",\"type\":\"uint256\"}],\"name\":\"setFees\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x877288c4e6eba4f635ba7428706447353b47de75_99cb12de",
        "{\"name\":\"Stash Factory\",\"address\":\"0x877288c4e6EbA4f635bA7428706447353B47De75\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_staker\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_stashVersion\",\"type\":\"uint256\"}],\"name\":\"CreateStash\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8798249c2e607446efb7ad49ec89dd1865ff4272_095ea7b3",
        "{\"name\":\"SushiBar\",\"address\":\"0x8798249c2E607446EfB7Ad49eC89dD1865Ff4272\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8798249c2e607446efb7ad49ec89dd1865ff4272_a457c2d7",
        "{\"name\":\"SushiBar\",\"address\":\"0x8798249c2E607446EfB7Ad49eC89dD1865Ff4272\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"subtractedValue\",\"type\":\"uint256\"}],\"name\":\"decreaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8798249c2e607446efb7ad49ec89dd1865ff4272_a59f3e0c",
        "{\"name\":\"SushiBar\",\"address\":\"0x8798249c2E607446EfB7Ad49eC89dD1865Ff4272\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"enter\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8798249c2e607446efb7ad49ec89dd1865ff4272_39509351",
        "{\"name\":\"SushiBar\",\"address\":\"0x8798249c2E607446EfB7Ad49eC89dD1865Ff4272\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"addedValue\",\"type\":\"uint256\"}],\"name\":\"increaseAllowance\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8798249c2e607446efb7ad49ec89dd1865ff4272_67dfd4c9",
        "{\"name\":\"SushiBar\",\"address\":\"0x8798249c2E607446EfB7Ad49eC89dD1865Ff4272\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_share\",\"type\":\"uint256\"}],\"name\":\"leave\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8798249c2e607446efb7ad49ec89dd1865ff4272_a9059cbb",
        "{\"name\":\"SushiBar\",\"address\":\"0x8798249c2E607446EfB7Ad49eC89dD1865Ff4272\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8798249c2e607446efb7ad49ec89dd1865ff4272_23b872dd",
        "{\"name\":\"SushiBar\",\"address\":\"0x8798249c2E607446EfB7Ad49eC89dD1865Ff4272\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"sender\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_69a7e57b",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"crToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minCrTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"crTokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_46d4b548",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minToTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_0dc9de85",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_715018a6",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_9735a634",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_3ff428c7",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_fbec27bf",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_01e980d4",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_550bfa56",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_1385d24c",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_f2fde38b",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x897e5bb1b7074cf6177d64d87f0bfb5e032b00b7_5ecb16cd",
        "{\"name\":\"Cream_Zap_V1\",\"address\":\"0x897E5Bb1B7074cf6177D64D87F0Bfb5E032b00b7\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8cb38a75ecd8572af23a8d086e9bda96ca521889_1a8dc5d3",
        "{\"name\":\"Claim Zap\",\"address\":\"0x8cB38a75eCd8572Af23a8D086e9bda96ca521889\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"rewardContracts\",\"type\":\"address[]\"},{\"internalType\":\"address[]\",\"name\":\"extraRewardContracts\",\"type\":\"address[]\"},{\"internalType\":\"bool\",\"name\":\"claimCvx\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"claimCvxStake\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"claimcvxCrv\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"lockCrvDeposit\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"depositCrvMaxAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minAmountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"depositCvxMaxAmount\",\"type\":\"uint256\"}],\"name\":\"claimRewards\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8cb38a75ecd8572af23a8d086e9bda96ca521889_8757b15b",
        "{\"name\":\"Claim Zap\",\"address\":\"0x8cB38a75eCd8572Af23a8D086e9bda96ca521889\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"setApprovals\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x8f7dd610c457fc7cb26b0f9db4e77581f94f70ac_2c1f83de",
        "{\"name\":\"Payable Proxy (operator)\",\"address\":\"0x8f7Dd610c457FC7Cb26B0f9Db4e77581f94F70aC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"enum Actions.ActionType\",\"name\":\"actionType\",\"type\":\"uint8\"},{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"secondAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"vaultId\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"index\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct Actions.ActionArgs[]\",\"name\":\"_actions\",\"type\":\"tuple[]\"},{\"internalType\":\"address payable\",\"name\":\"_sendEthTo\",\"type\":\"address\"}],\"name\":\"operate\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_38b32e68",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toVault\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"superVault\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"isAaveUnderlying\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"minYVTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"intermediateToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensReceived\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_0dc9de85",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_715018a6",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_9735a634",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_3ff428c7",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_fbec27bf",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_01e980d4",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_550bfa56",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_1385d24c",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_f2fde38b",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x92be6adb6a12da0ca607f9d87db2f9978cd6ec3e_5ecb16cd",
        "{\"name\":\"Yearn yVault Add\",\"address\":\"0x92Be6ADB6a12Da0CA607F9d87DB2F9978cD6ec3E\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_3fe9bc06",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"}],\"name\":\"claimCrv\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_2dbfa735",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_distroContract\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_token\",\"type\":\"address\"}],\"name\":\"claimFees\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_ef5cfb8c",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"}],\"name\":\"claimRewards\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_b52c05fe",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_unlockTime\",\"type\":\"uint256\"}],\"name\":\"createLock\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_f9609f08",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"}],\"name\":\"deposit\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_b61d27f6",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"execute\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"},{\"internalType\":\"bytes\",\"name\":\"\",\"type\":\"bytes\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_15456eba",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"increaseAmount\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_3c9a2a1a",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"increaseTime\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_86d1a69f",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"release\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_f2c098b7",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_depositor\",\"type\":\"address\"}],\"name\":\"setDepositor\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_b3ab15fb",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_operator\",\"type\":\"address\"}],\"name\":\"setOperator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_13af4035",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_owner\",\"type\":\"address\"}],\"name\":\"setOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_fa3964b2",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_stash\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"setStashAccess\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_e2cdd42a",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_voteId\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_votingAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_support\",\"type\":\"bool\"}],\"name\":\"vote\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_5d7e9bcb",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_weight\",\"type\":\"uint256\"}],\"name\":\"voteGaugeWeight\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_51cff8d9",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"_asset\",\"type\":\"address\"}],\"name\":\"withdraw\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"balance\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_d9caed12",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x989aeb4d175e16225e39e87d0d97a3360524ad80_09cae2c8",
        "{\"name\":\"Voter Proxy\",\"address\":\"0x989AEb4d175e16225E39E87d0D97A3360524AD80\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"}],\"name\":\"withdrawAll\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_c0a8346e",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"vault\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minVaultTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"intermediateToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensReceived\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_0dc9de85",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_715018a6",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_9735a634",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_3ff428c7",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_fbec27bf",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_01e980d4",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_550bfa56",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_1385d24c",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_f2fde38b",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x98e562a87c5243eef333e43cb1834223f526c434_5ecb16cd",
        "{\"name\":\"Harvest Add\",\"address\":\"0x98E562a87c5243eeF333E43cb1834223f526c434\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_e9fad8ee",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"exit\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_3d18b912",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"getReward\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_3c6b16ab",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"reward\",\"type\":\"uint256\"}],\"name\":\"notifyRewardAmount\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_fa9389a2",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"pushReward\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_715018a6",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_0d68b761",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_rewardDistribution\",\"type\":\"address\"}],\"name\":\"setRewardDistribution\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_9137c1a7",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_store\",\"type\":\"address\"}],\"name\":\"setStorage\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_a694fc3a",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"stake\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_f2fde38b",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf_2e1a7d4d",
        "{\"name\":\"Harvest Finance\",\"address\":\"0x99b0d6641a63ce173e6eb063b3d3aed9a35cf9bf\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48_3659cfe6",
        "{\"name\":\"USDC\",\"address\":\"0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newImplementation\",\"type\":\"address\"}],\"name\":\"upgradeTo\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48_4f1ef286",
        "{\"name\":\"USDC\",\"address\":\"0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newImplementation\",\"type\":\"address\"},{\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"upgradeToAndCall\",\"outputs\":[],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48_8f283970",
        "{\"name\":\"USDC\",\"address\":\"0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"newAdmin\",\"type\":\"address\"}],\"name\":\"changeAdmin\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa14eeefa753a166a5651bce7b84094f614df0d05_b9e3adaa",
        "{\"name\":\"Bancor Add\",\"address\":\"0xa14EEefa753a166a5651bce7B84094f614Df0D05\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_fromTokenAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_toBanConverterAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_toReserveTokenAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_allowanceTarget\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"}],\"name\":\"ZapInSingleSided\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"lptReceived\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"id\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa14eeefa753a166a5651bce7b84094f614df0d05_551196d5",
        "{\"name\":\"Bancor Add\",\"address\":\"0xa14EEefa753a166a5651bce7B84094f614Df0D05\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_tokenAddress\",\"type\":\"address\"}],\"name\":\"inCaseTokengetsStuck\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa14eeefa753a166a5651bce7b84094f614df0d05_715018a6",
        "{\"name\":\"Bancor Add\",\"address\":\"0xa14EEefa753a166a5651bce7B84094f614Df0D05\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa14eeefa753a166a5651bce7b84094f614df0d05_b10e1dbc",
        "{\"name\":\"Bancor Add\",\"address\":\"0xa14EEefa753a166a5651bce7B84094f614Df0D05\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint16\",\"name\":\"_new_goodwill\",\"type\":\"uint16\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa14eeefa753a166a5651bce7b84094f614df0d05_1385d24c",
        "{\"name\":\"Bancor Add\",\"address\":\"0xa14EEefa753a166a5651bce7B84094f614Df0D05\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa14eeefa753a166a5651bce7b84094f614df0d05_f2fde38b",
        "{\"name\":\"Bancor Add\",\"address\":\"0xa14EEefa753a166a5651bce7B84094f614Df0D05\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa14eeefa753a166a5651bce7b84094f614df0d05_3ccfd60b",
        "{\"name\":\"Bancor Add\",\"address\":\"0xa14EEefa753a166a5651bce7B84094f614Df0D05\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"withdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa1bc2cf69d474b39b91665e24e7f2606ed142991_769f42e7",
        "{\"name\":\"Airdrop Factory\",\"address\":\"0xa1Bc2Cf69D474b39B91665e24E7f2606Ed142991\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"CreateMerkleAirdrop\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5644e29708357803b5a882d272c41cc0df92b34_13ead562",
        "{\"name\":\"Uniswap V3_Migrator\",\"address\":\"0xa5644e29708357803b5a882d272c41cc0df92b34\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"token1\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"uint160\",\"name\":\"sqrtPriceX96\",\"type\":\"uint160\"}],\"name\":\"createAndInitializePoolIfNecessary\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"pool\",\"type\":\"address\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5644e29708357803b5a882d272c41cc0df92b34_81c88719",
        "{\"name\":\"Uniswap V3_Migrator\",\"address\":\"0xa5644e29708357803b5a882d272c41cc0df92b34\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"pair\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidityToMigrate\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"percentageToMigrate\",\"type\":\"uint8\"},{\"internalType\":\"address\",\"name\":\"token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"token1\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"int24\",\"name\":\"tickLower\",\"type\":\"int24\"},{\"internalType\":\"int24\",\"name\":\"tickUpper\",\"type\":\"int24\"},{\"internalType\":\"uint256\",\"name\":\"amount0Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Min\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"refundAsETH\",\"type\":\"bool\"}],\"internalType\":\"struct IV3Migrator.MigrateParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"migrate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5644e29708357803b5a882d272c41cc0df92b34_ac9650d8",
        "{\"name\":\"Uniswap V3_Migrator\",\"address\":\"0xa5644e29708357803b5a882d272c41cc0df92b34\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes[]\",\"name\":\"data\",\"type\":\"bytes[]\"}],\"name\":\"multicall\",\"outputs\":[{\"internalType\":\"bytes[]\",\"name\":\"results\",\"type\":\"bytes[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5644e29708357803b5a882d272c41cc0df92b34_f3995c67",
        "{\"name\":\"Uniswap V3_Migrator\",\"address\":\"0xa5644e29708357803b5a882d272c41cc0df92b34\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5644e29708357803b5a882d272c41cc0df92b34_4659a494",
        "{\"name\":\"Uniswap V3_Migrator\",\"address\":\"0xa5644e29708357803b5a882d272c41cc0df92b34\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowed\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5644e29708357803b5a882d272c41cc0df92b34_a4a78f0c",
        "{\"name\":\"Uniswap V3_Migrator\",\"address\":\"0xa5644e29708357803b5a882d272c41cc0df92b34\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowedIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5644e29708357803b5a882d272c41cc0df92b34_c2e3140a",
        "{\"name\":\"Uniswap V3_Migrator\",\"address\":\"0xa5644e29708357803b5a882d272c41cc0df92b34\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_ebd31e8e",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_callee\",\"type\":\"address\"}],\"name\":\"blacklistCallee\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_15f73b01",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"}],\"name\":\"blacklistCollateral\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_79df1e44",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_otokenAddress\",\"type\":\"address\"}],\"name\":\"blacklistOtoken\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_a6dfe83f",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_underlying\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_strike\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_isPut\",\"type\":\"bool\"}],\"name\":\"blacklistProduct\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_715018a6",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_f2fde38b",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_708a0393",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_callee\",\"type\":\"address\"}],\"name\":\"whitelistCallee\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_a34626c4",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"}],\"name\":\"whitelistCollateral\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_ec7127b6",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_otokenAddress\",\"type\":\"address\"}],\"name\":\"whitelistOtoken\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa5ea18ac6865f315ff5dd9f1a7fb1d41a30a6779_82d90ebf",
        "{\"name\":\"Whitelist\",\"address\":\"0xa5EA18ac6865f315ff5dD9f1a7fb1d41A30a6779\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_underlying\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_strike\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_isPut\",\"type\":\"bool\"}],\"name\":\"whitelistProduct\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xa6b71e26c5e0845f74c812102ca7114b6a896ab2_2500510e",
        "{\"name\":\"Gnosis Safe: Proxy Factory 1.3.0\",\"address\":\"0xa6b71e26c5e0845f74c812102ca7114b6a896ab2\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_singleton\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"initializer\",\"type\":\"bytes\"},{\"internalType\":\"uint256\",\"name\":\"saltNonce\",\"type\":\"uint256\"}],\"name\":\"calculateCreateProxyWithNonceAddress\",\"outputs\":[{\"internalType\":\"contract GnosisSafeProxy\",\"name\":\"proxy\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xa6b71e26c5e0845f74c812102ca7114b6a896ab2_61b69abd",
        "{\"name\":\"Gnosis Safe: Proxy Factory 1.3.0\",\"address\":\"0xa6b71e26c5e0845f74c812102ca7114b6a896ab2\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"singleton\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"createProxy\",\"outputs\":[{\"internalType\":\"contract GnosisSafeProxy\",\"name\":\"proxy\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xa6b71e26c5e0845f74c812102ca7114b6a896ab2_d18af54d",
        "{\"name\":\"Gnosis Safe: Proxy Factory 1.3.0\",\"address\":\"0xa6b71e26c5e0845f74c812102ca7114b6a896ab2\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_singleton\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"initializer\",\"type\":\"bytes\"},{\"internalType\":\"uint256\",\"name\":\"saltNonce\",\"type\":\"uint256\"},{\"internalType\":\"contract IProxyCreationCallback\",\"name\":\"callback\",\"type\":\"address\"}],\"name\":\"createProxyWithCallback\",\"outputs\":[{\"internalType\":\"contract GnosisSafeProxy\",\"name\":\"proxy\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xa6b71e26c5e0845f74c812102ca7114b6a896ab2_1688f0b9",
        "{\"name\":\"Gnosis Safe: Proxy Factory 1.3.0\",\"address\":\"0xa6b71e26c5e0845f74c812102ca7114b6a896ab2\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_singleton\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"initializer\",\"type\":\"bytes\"},{\"internalType\":\"uint256\",\"name\":\"saltNonce\",\"type\":\"uint256\"}],\"name\":\"createProxyWithNonce\",\"outputs\":[{\"internalType\":\"contract GnosisSafeProxy\",\"name\":\"proxy\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xb440dd674e1243644791a4adfe3a2abb0a92d309_1627540c",
        "{\"name\":\"Synthetix\",\"address\":\"0xb440dd674e1243644791a4adfe3a2abb0a92d309\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_owner\",\"type\":\"address\"}],\"name\":\"nominateNewOwner\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xb440dd674e1243644791a4adfe3a2abb0a92d309_776d1a01",
        "{\"name\":\"Synthetix\",\"address\":\"0xb440dd674e1243644791a4adfe3a2abb0a92d309\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"_target\",\"type\":\"address\"}],\"name\":\"setTarget\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xb440dd674e1243644791a4adfe3a2abb0a92d309_79ba5097",
        "{\"name\":\"Synthetix\",\"address\":\"0xb440dd674e1243644791a4adfe3a2abb0a92d309\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"acceptOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xb440dd674e1243644791a4adfe3a2abb0a92d309_907dff97",
        "{\"name\":\"Synthetix\",\"address\":\"0xb440dd674e1243644791a4adfe3a2abb0a92d309\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"callData\",\"type\":\"bytes\"},{\"name\":\"numTopics\",\"type\":\"uint256\"},{\"name\":\"topic1\",\"type\":\"bytes32\"},{\"name\":\"topic2\",\"type\":\"bytes32\"},{\"name\":\"topic3\",\"type\":\"bytes32\"},{\"name\":\"topic4\",\"type\":\"bytes32\"}],\"name\":\"_emit\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xb440dd674e1243644791a4adfe3a2abb0a92d309_befff6af",
        "{\"name\":\"Synthetix\",\"address\":\"0xb440dd674e1243644791a4adfe3a2abb0a92d309\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"value\",\"type\":\"bool\"}],\"name\":\"setUseDELEGATECALL\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_095ea7b3",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_7d17fcbe",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"emergencySetStartingIndexBlock\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_34918dfd",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"flipSaleState\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_a723533e",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"numberOfTokens\",\"type\":\"uint256\"}],\"name\":\"mintApe\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_715018a6",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_b0f67427",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"reserveApes\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_42842e0e",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"}],\"name\":\"safeTransferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_b88d4fde",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"safeTransferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_a22cb465",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"operator\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"approved\",\"type\":\"bool\"}],\"name\":\"setApprovalForAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_55f804b3",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"string\",\"name\":\"baseURI\",\"type\":\"string\"}],\"name\":\"setBaseURI\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_10969523",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"string\",\"name\":\"provenanceHash\",\"type\":\"string\"}],\"name\":\"setProvenanceHash\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_018a2c37",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"revealTimeStamp\",\"type\":\"uint256\"}],\"name\":\"setRevealTimestamp\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_e9866550",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"setStartingIndex\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_23b872dd",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_f2fde38b",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d_3ccfd60b",
        "{\"name\":\"BoredApeYachtClub\",\"address\":\"0xbc4ca0eda7647a8ab7c2061c2e118a18a936f13d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"withdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2_095ea7b3",
        "{\"name\":\"WETH\",\"address\":\"0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"guy\",\"type\":\"address\"},{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2_23b872dd",
        "{\"name\":\"WETH\",\"address\":\"0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"src\",\"type\":\"address\"},{\"name\":\"dst\",\"type\":\"address\"},{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2_2e1a7d4d",
        "{\"name\":\"WETH\",\"address\":\"0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2_a9059cbb",
        "{\"name\":\"WETH\",\"address\":\"0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"name\":\"dst\",\"type\":\"address\"},{\"name\":\"wad\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2_d0e30db0",
        "{\"name\":\"WETH\",\"address\":\"0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"deposit\",\"outputs\":[],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc0aee478e3658e2610c5f7a4a2e1777ce9e4f2ac_c9c65396",
        "{\"name\":\"SushiSwap factory\",\"address\":\"0xC0AEe478e3658e2610c5F7A4A2E1777cE9e4f2Ac\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"tokenA\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenB\",\"type\":\"address\"}],\"name\":\"createPair\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"pair\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc0aee478e3658e2610c5f7a4a2e1777ce9e4f2ac_f46901ed",
        "{\"name\":\"SushiSwap factory\",\"address\":\"0xC0AEe478e3658e2610c5F7A4A2E1777cE9e4f2Ac\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_feeTo\",\"type\":\"address\"}],\"name\":\"setFeeTo\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc0aee478e3658e2610c5f7a4a2e1777ce9e4f2ac_a2e74af6",
        "{\"name\":\"SushiSwap factory\",\"address\":\"0xC0AEe478e3658e2610c5F7A4A2E1777cE9e4f2Ac\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_feeToSetter\",\"type\":\"address\"}],\"name\":\"setFeeToSetter\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc0aee478e3658e2610c5f7a4a2e1777ce9e4f2ac_23cf3118",
        "{\"name\":\"SushiSwap factory\",\"address\":\"0xC0AEe478e3658e2610c5F7A4A2E1777cE9e4f2Ac\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_migrator\",\"type\":\"address\"}],\"name\":\"setMigrator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_1eaaa045",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_allocPoint\",\"type\":\"uint256\"},{\"internalType\":\"contract IERC20\",\"name\":\"_lpToken\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_withUpdate\",\"type\":\"bool\"}],\"name\":\"add\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_e2bbb158",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_8d88a90e",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_devaddr\",\"type\":\"address\"}],\"name\":\"dev\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_5312ea8e",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"emergencyWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_630b5ba1",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"massUpdatePools\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_454b0608",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"migrate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_715018a6",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_64482f79",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_allocPoint\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"_withUpdate\",\"type\":\"bool\"}],\"name\":\"set\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_23cf3118",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IMigratorChef\",\"name\":\"_migrator\",\"type\":\"address\"}],\"name\":\"setMigrator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_f2fde38b",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_51eb05a6",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"updatePool\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc2edad668740f1aa35e4d8f227fb8e17dca888cd_441a3e70",
        "{\"name\":\"SushiSwap MasterChef\",\"address\":\"0xc2edad668740f1aa35e4d8f227fb8e17dca888cd\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_095ea7b3",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_42966c68",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"}],\"name\":\"burn\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_302e5bb1",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint128\",\"name\":\"amount0Max\",\"type\":\"uint128\"},{\"internalType\":\"uint128\",\"name\":\"amount1Max\",\"type\":\"uint128\"}],\"internalType\":\"struct INonfungiblePositionManager.CollectParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"collect\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amount0\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_13ead562",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"token1\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"uint160\",\"name\":\"sqrtPriceX96\",\"type\":\"uint160\"}],\"name\":\"createAndInitializePoolIfNecessary\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"pool\",\"type\":\"address\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_c6887a9d",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"uint128\",\"name\":\"liquidity\",\"type\":\"uint128\"},{\"internalType\":\"uint256\",\"name\":\"amount0Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"internalType\":\"struct INonfungiblePositionManager.DecreaseLiquidityParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"decreaseLiquidity\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amount0\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_f13884c1",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount0Desired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Desired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount0Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"internalType\":\"struct INonfungiblePositionManager.IncreaseLiquidityParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"increaseLiquidity\",\"outputs\":[{\"internalType\":\"uint128\",\"name\":\"liquidity\",\"type\":\"uint128\"},{\"internalType\":\"uint256\",\"name\":\"amount0\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_4405fca9",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"token1\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"int24\",\"name\":\"tickLower\",\"type\":\"int24\"},{\"internalType\":\"int24\",\"name\":\"tickUpper\",\"type\":\"int24\"},{\"internalType\":\"uint256\",\"name\":\"amount0Desired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Desired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount0Min\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Min\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"internalType\":\"struct INonfungiblePositionManager.MintParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"mint\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"uint128\",\"name\":\"liquidity\",\"type\":\"uint128\"},{\"internalType\":\"uint256\",\"name\":\"amount0\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_ac9650d8",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes[]\",\"name\":\"data\",\"type\":\"bytes[]\"}],\"name\":\"multicall\",\"outputs\":[{\"internalType\":\"bytes[]\",\"name\":\"results\",\"type\":\"bytes[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_7ac2ff7b",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"permit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_12210e8a",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"refundETH\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_42842e0e",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"}],\"name\":\"safeTransferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_b88d4fde",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"safeTransferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_f3995c67",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_4659a494",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowed\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_a4a78f0c",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowedIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_c2e3140a",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_a22cb465",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"operator\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"approved\",\"type\":\"bool\"}],\"name\":\"setApprovalForAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_df2ab5bb",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"sweepToken\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_23b872dd",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"tokenId\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_d3487997",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amount0Owed\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount1Owed\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"uniswapV3MintCallback\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc36442b4a4522e871399cd717abdd847ab11fe88_49404b7c",
        "{\"name\":\"Uniswap V3_Positions NFT\",\"address\":\"0xc36442b4a4522e871399cd717abdd847ab11fe88\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"unwrapWETH9\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_238efcbc",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"acceptGovernance\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_095ea7b3",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_4e71d92d",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"claim\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_ddeae033",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"claimFor\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_5c19a95c",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"delegatee\",\"type\":\"address\"}],\"name\":\"delegate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_c3cda520",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"delegatee\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"delegateBySig\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_b6b55f25",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_de5f6268",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"depositAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_d505accf",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"spender\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"permit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_1919db33",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_feeDistribution\",\"type\":\"address\"}],\"name\":\"setFeeDistribution\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_ab033ea9",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_governance\",\"type\":\"address\"}],\"name\":\"setGovernance\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_97107d6d",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_proxy\",\"type\":\"address\"}],\"name\":\"setProxy\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_a9059cbb",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"dst\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_23b872dd",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"src\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"dst\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_a2e62045",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"update\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc5bddf9843308380375a611c18b50fb9341f502a_0e0a5968",
        "{\"name\":\"veCurveVault\",\"address\":\"0xc5bDdf9843308380375a611c18B50Fb9341f502A\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"updateFor\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_28932094",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toPJar\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minPJarTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"intermediateToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensReceived\",\"type\":\"uint256\"}],\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_0dc9de85",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_715018a6",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_9735a634",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_3ff428c7",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_fbec27bf",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_01e980d4",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_550bfa56",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_1385d24c",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_f2fde38b",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xc695f73c1862e050059367b2e64489e66c525983_5ecb16cd",
        "{\"name\":\"Pickle Add\",\"address\":\"0xc695f73c1862e050059367B2E64489E66c525983\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_5e43c47b",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_reward\",\"type\":\"address\"}],\"name\":\"addExtraReward\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_0569d388",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"clearExtraRewards\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_f14faf6f",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"donate\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_a4698feb",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bool\",\"name\":\"_stake\",\"type\":\"bool\"}],\"name\":\"getReward\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_f9a6e764",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_account\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_claimExtras\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"_stake\",\"type\":\"bool\"}],\"name\":\"getReward\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_590a41f5",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_rewards\",\"type\":\"uint256\"}],\"name\":\"queueNewRewards\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_a694fc3a",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"stake\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_8dcb4061",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"stakeAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_2ee40908",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_for\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"stakeFor\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_38d07436",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"claim\",\"type\":\"bool\"}],\"name\":\"withdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xcf50b810e57ac33b91dcf525c6ddd9881b139332_1c1c6fe5",
        "{\"name\":\"CVX Rewards\",\"address\":\"0xCF50b810E57Ac33B91dCF525C6ddd9881B139332\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bool\",\"name\":\"claim\",\"type\":\"bool\"}],\"name\":\"withdrawAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_89c6973b",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromVault\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"isAaveUnderlying\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"minToTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensReceived\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_2c6ca9ac",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromVault\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"isAaveUnderlying\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"minToTokens\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"permitSig\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapOutWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensReceived\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_0dc9de85",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_715018a6",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_9735a634",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_3ff428c7",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_fbec27bf",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_01e980d4",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_550bfa56",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_1385d24c",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_f2fde38b",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd6b88257e91e4e4d4e990b3a858c849ef2dfde8c_5ecb16cd",
        "{\"name\":\"Yearn yVault Remove\",\"address\":\"0xd6b88257e91e4E4D4E990B3A858c849EF2DFdE8c\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_d8f6eb5b",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"swapAddress\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"incomingCrv\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"intermediateToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minToTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"_swapCallData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"ToTokensBought\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_0dc9de85",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_715018a6",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_9735a634",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_3ff428c7",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_fbec27bf",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_01e980d4",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_550bfa56",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_1385d24c",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_f2fde38b",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_e5953382",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"contract Curve_Registry_V2\",\"name\":\"newCurveRegistry\",\"type\":\"address\"}],\"name\":\"updateCurveRegistry\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd754ffb697cd276ad6b0f0273521775c1a11b175_5ecb16cd",
        "{\"name\":\"Curve Remove\",\"address\":\"0xD754ffB697CD276ad6B0f0273521775C1A11b175\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":false,\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_0d582f13",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"addOwnerWithThreshold\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_d4d9bdcd",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"hashToApprove\",\"type\":\"bytes32\"}],\"name\":\"approveHash\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_694e80c3",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"changeThreshold\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_e009cfde",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevModule\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"disableModule\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_610b5925",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"enableModule\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_6a761202",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"},{\"internalType\":\"uint256\",\"name\":\"safeTxGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"baseGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"gasPrice\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"gasToken\",\"type\":\"address\"},{\"internalType\":\"address payable\",\"name\":\"refundReceiver\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"signatures\",\"type\":\"bytes\"}],\"name\":\"execTransaction\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_468721a7",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"execTransactionFromModule\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_5229073f",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"execTransactionFromModuleReturnData\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"},{\"internalType\":\"bytes\",\"name\":\"returnData\",\"type\":\"bytes\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_f8dc5dd9",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"removeOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_c4ca3a9c",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"requiredTxGas\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_f08a0323",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"handler\",\"type\":\"address\"}],\"name\":\"setFallbackHandler\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_e19a9dd9",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"guard\",\"type\":\"address\"}],\"name\":\"setGuard\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_b63e800d",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"_owners\",\"type\":\"address[]\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"fallbackHandler\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"paymentToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"payment\",\"type\":\"uint256\"},{\"internalType\":\"address payable\",\"name\":\"paymentReceiver\",\"type\":\"address\"}],\"name\":\"setup\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_b4faba09",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"targetContract\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"calldataPayload\",\"type\":\"bytes\"}],\"name\":\"simulateAndRevert\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9db270c1b5e3bd161e8c8503c55ceabee709552_e318b52b",
        "{\"name\":\"Gnosis Safe: Singleton 1.3.0\",\"address\":\"0xd9db270c1b5e3bd161e8c8503c55ceabee709552\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"oldOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"swapOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}}}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_e8e33700",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"tokenA\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenB\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountADesired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBDesired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountAMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"addLiquidity\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_f305d719",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenDesired\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"addLiquidityETH\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountToken\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_baa2abde",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"tokenA\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenB\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountAMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"removeLiquidity\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_02751cec",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"removeLiquidityETH\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountToken\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_af2979eb",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"removeLiquidityETHSupportingFeeOnTransferTokens\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_ded9382a",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"approveMax\",\"type\":\"bool\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"removeLiquidityETHWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountToken\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_5b0d5984",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountTokenMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountETHMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"approveMax\",\"type\":\"bool\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"removeLiquidityETHWithPermitSupportingFeeOnTransferTokens\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountETH\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_2195995c",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"tokenA\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenB\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"liquidity\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountAMin\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountBMin\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"approveMax\",\"type\":\"bool\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"removeLiquidityWithPermit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountA\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountB\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_fb3bdb41",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapETHForExactTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_7ff36ab5",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactETHForTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_b6f9de95",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactETHForTokensSupportingFeeOnTransferTokens\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_18cbafe5",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForETH\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_791ac947",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForETHSupportingFeeOnTransferTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_38ed1739",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_5c11d795",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMin\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapExactTokensForTokensSupportingFeeOnTransferTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_4a25d94a",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMax\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapTokensForExactETH\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd9e1ce17f2641f24ae83637ab66a2cca9c378b9f_8803dbee",
        "{\"name\":\"SushiSwap Rotuer\",\"address\":\"0xd9e1cE17f2641f24aE83637ab66a2cca9C378B9F\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMax\",\"type\":\"uint256\"},{\"internalType\":\"address[]\",\"name\":\"path\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"}],\"name\":\"swapTokensForExactTokens\",\"outputs\":[{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_8fd3ab80",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"migrate\",\"outputs\":[{\"internalType\":\"bytes4\",\"name\":\"success\",\"type\":\"bytes4\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_d9627aa4",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20TokenV06[]\",\"name\":\"tokens\",\"type\":\"address[]\"},{\"internalType\":\"uint256\",\"name\":\"sellAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minBuyAmount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"isSushi\",\"type\":\"bool\"}],\"name\":\"sellToUniswap\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"buyAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_7e04ad9a",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"},{\"internalType\":\"uint256\",\"name\":\"sellAmount\",\"type\":\"uint256\"},{\"components\":[{\"internalType\":\"bytes4\",\"name\":\"selector\",\"type\":\"bytes4\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct IMultiplexFeature.WrappedMultiHopCall[]\",\"name\":\"calls\",\"type\":\"tuple[]\"}],\"internalType\":\"struct IMultiplexFeature.MultiHopFillData\",\"name\":\"fillData\",\"type\":\"tuple\"},{\"internalType\":\"uint256\",\"name\":\"totalEth\",\"type\":\"uint256\"}],\"name\":\"_multiHopFill\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"outputTokenAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"remainingEth\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_e6f90561",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"inputToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"outputToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"inputTokenAmount\",\"type\":\"uint256\"},{\"internalType\":\"contract ILiquidityProvider\",\"name\":\"provider\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"auxiliaryData\",\"type\":\"bytes\"}],\"name\":\"_sellToLiquidityProvider\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"outputTokenAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_d64d051a",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"},{\"internalType\":\"uint256\",\"name\":\"sellAmount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"isSushi\",\"type\":\"bool\"},{\"internalType\":\"address\",\"name\":\"pairAddress\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"_sellToUniswap\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"outputTokenAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_5447275d",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"inputToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"outputToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"sellAmount\",\"type\":\"uint256\"},{\"components\":[{\"internalType\":\"bytes4\",\"name\":\"selector\",\"type\":\"bytes4\"},{\"internalType\":\"uint256\",\"name\":\"sellAmount\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct IMultiplexFeature.WrappedBatchCall[]\",\"name\":\"calls\",\"type\":\"tuple[]\"}],\"internalType\":\"struct IMultiplexFeature.BatchFillData\",\"name\":\"fillData\",\"type\":\"tuple\"},{\"internalType\":\"uint256\",\"name\":\"minBuyAmount\",\"type\":\"uint256\"}],\"name\":\"batchFill\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"outputTokenAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_8fd3ab80",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"migrate\",\"outputs\":[{\"internalType\":\"bytes4\",\"name\":\"success\",\"type\":\"bytes4\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_4a4d430c",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"},{\"internalType\":\"uint256\",\"name\":\"sellAmount\",\"type\":\"uint256\"},{\"components\":[{\"internalType\":\"bytes4\",\"name\":\"selector\",\"type\":\"bytes4\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct IMultiplexFeature.WrappedMultiHopCall[]\",\"name\":\"calls\",\"type\":\"tuple[]\"}],\"internalType\":\"struct IMultiplexFeature.MultiHopFillData\",\"name\":\"fillData\",\"type\":\"tuple\"},{\"internalType\":\"uint256\",\"name\":\"minBuyAmount\",\"type\":\"uint256\"}],\"name\":\"multiHopFill\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"outputTokenAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_cad64df9",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address payable\",\"name\":\"taker\",\"type\":\"address\"},{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"inputToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"outputToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"inputTokenAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minOutputTokenAmount\",\"type\":\"uint256\"},{\"components\":[{\"internalType\":\"uint32\",\"name\":\"deploymentNonce\",\"type\":\"uint32\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct ITransformERC20Feature.Transformation[]\",\"name\":\"transformations\",\"type\":\"tuple[]\"}],\"internalType\":\"struct ITransformERC20Feature.TransformERC20Args\",\"name\":\"args\",\"type\":\"tuple\"}],\"name\":\"_transformERC20\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"outputTokenAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_287b071b",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"createTransformWallet\",\"outputs\":[{\"internalType\":\"contract IFlashWallet\",\"name\":\"wallet\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_ce5494bb",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"transformerDeployer\",\"type\":\"address\"}],\"name\":\"migrate\",\"outputs\":[{\"internalType\":\"bytes4\",\"name\":\"success\",\"type\":\"bytes4\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_56ce180a",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"quoteSigner\",\"type\":\"address\"}],\"name\":\"setQuoteSigner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_87c96419",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"transformerDeployer\",\"type\":\"address\"}],\"name\":\"setTransformerDeployer\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xdef1c0ded9bec7f1a1670819833240f027b25eff_5b65ff6a",
        "{\"name\":\"ZeroEx\",\"address\":\"0xdef1c0ded9bec7f1a1670819833240f027b25eff\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"inputToken\",\"type\":\"address\"},{\"internalType\":\"contract IERC20TokenV06\",\"name\":\"outputToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"inputTokenAmount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"minOutputTokenAmount\",\"type\":\"uint256\"},{\"components\":[{\"internalType\":\"uint32\",\"name\":\"deploymentNonce\",\"type\":\"uint32\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"internalType\":\"struct ITransformERC20Feature.Transformation[]\",\"name\":\"transformations\",\"type\":\"tuple[]\"}],\"name\":\"transformERC20\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"outputTokenAmount\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50_4e71e0c8",
        "{\"name\":\"SushiMaker\",\"address\":\"0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"claimOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50_bd1b820c",
        "{\"name\":\"SushiMaker\",\"address\":\"0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token0\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"token1\",\"type\":\"address\"}],\"name\":\"convert\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50_303e6aa4",
        "{\"name\":\"SushiMaker\",\"address\":\"0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"token0\",\"type\":\"address[]\"},{\"internalType\":\"address[]\",\"name\":\"token1\",\"type\":\"address[]\"}],\"name\":\"convertMultiple\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50_9d22ae8c",
        "{\"name\":\"SushiMaker\",\"address\":\"0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"bridge\",\"type\":\"address\"}],\"name\":\"setBridge\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50_078dfbe7",
        "{\"name\":\"SushiMaker\",\"address\":\"0xe11fc0b43ab98eb91e9836129d1ee7c3bc95df50\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"direct\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"renounce\",\"type\":\"bool\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_510e92f3",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"uint256[2]\",\"name\":\"swapAmounts\",\"type\":\"uint256[2]\"},{\"internalType\":\"uint256[2]\",\"name\":\"minTokensRec\",\"type\":\"uint256[2]\"},{\"internalType\":\"address[2]\",\"name\":\"swapTargets\",\"type\":\"address[2]\"},{\"internalType\":\"bytes[2]\",\"name\":\"swapData\",\"type\":\"bytes[2]\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapBridge\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_0dc9de85",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_715018a6",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_9735a634",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_3ff428c7",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_fbec27bf",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_01e980d4",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_550bfa56",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_1385d24c",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_f2fde38b",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe34b087bf3c99e664316a15b01e5295eb3512760_5ecb16cd",
        "{\"name\":\"Ethereum to Polygon Bridge\",\"address\":\"0xe34B087Bf3C99E664316A15B01E5295eB3512760\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_80fb3ad6",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"bytes\",\"name\":\"path\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMinimum\",\"type\":\"uint256\"}],\"internalType\":\"struct ISwapRouter.ExactInputParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactInput\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_5d76b977",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"tokenIn\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenOut\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOutMinimum\",\"type\":\"uint256\"},{\"internalType\":\"uint160\",\"name\":\"sqrtPriceLimitX96\",\"type\":\"uint160\"}],\"internalType\":\"struct ISwapRouter.ExactInputSingleParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactInputSingle\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_d42bbb58",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"bytes\",\"name\":\"path\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMaximum\",\"type\":\"uint256\"}],\"internalType\":\"struct ISwapRouter.ExactOutputParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactOutput\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_5bd7800f",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"components\":[{\"internalType\":\"address\",\"name\":\"tokenIn\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"tokenOut\",\"type\":\"address\"},{\"internalType\":\"uint24\",\"name\":\"fee\",\"type\":\"uint24\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amountInMaximum\",\"type\":\"uint256\"},{\"internalType\":\"uint160\",\"name\":\"sqrtPriceLimitX96\",\"type\":\"uint160\"}],\"internalType\":\"struct ISwapRouter.ExactOutputSingleParams\",\"name\":\"params\",\"type\":\"tuple\"}],\"name\":\"exactOutputSingle\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_ac9650d8",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes[]\",\"name\":\"data\",\"type\":\"bytes[]\"}],\"name\":\"multicall\",\"outputs\":[{\"internalType\":\"bytes[]\",\"name\":\"results\",\"type\":\"bytes[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_12210e8a",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"refundETH\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_f3995c67",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_4659a494",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowed\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_a4a78f0c",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"nonce\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"expiry\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitAllowedIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_c2e3140a",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"selfPermitIfNecessary\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_df2ab5bb",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"sweepToken\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_e0e189a0",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"feeBips\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"feeRecipient\",\"type\":\"address\"}],\"name\":\"sweepTokenWithFee\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_fa461e33",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"int256\",\"name\":\"amount0Delta\",\"type\":\"int256\"},{\"internalType\":\"int256\",\"name\":\"amount1Delta\",\"type\":\"int256\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"uniswapV3SwapCallback\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_49404b7c",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"}],\"name\":\"unwrapWETH9\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe592427a0aece92de3edee1f18e0157c05861564_9b2c0a37",
        "{\"name\":\"Uniswap V3_Router\",\"address\":\"0xe592427a0aece92de3edee1f18e0157c05861564\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"amountMinimum\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"recipient\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"feeBips\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"feeRecipient\",\"type\":\"address\"}],\"name\":\"unwrapWETH9WithFee\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_69a7e57b",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"cToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minCtokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"cTokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_46d4b548",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minToTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"}],\"name\":\"ZapOut\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensRec\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_0dc9de85",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_715018a6",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_9735a634",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_3ff428c7",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_fbec27bf",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_01e980d4",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_550bfa56",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_1385d24c",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_f2fde38b",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe5e8506a590766d738d80affc6b5e538c4b92f82_5ecb16cd",
        "{\"name\":\"Compound_Zap_V1\",\"address\":\"0xE5E8506a590766d738D80aFfc6b5e538C4B92F82\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_c0a8346e",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"fromToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amountIn\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"toVault\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"minMVTokens\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"intermediateToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"swapTarget\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"swapData\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"shouldSellEntireBalance\",\"type\":\"bool\"}],\"name\":\"ZapIn\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"tokensReceived\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_0dc9de85",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"affilliateWithdraw\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_715018a6",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_9735a634",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"targets\",\"type\":\"address[]\"},{\"internalType\":\"bool[]\",\"name\":\"isApproved\",\"type\":\"bool[]\"}],\"name\":\"setApprovedTargets\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_3ff428c7",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_affiliate\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"set_affiliate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_fbec27bf",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"zapAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"status\",\"type\":\"bool\"}],\"name\":\"set_feeWhitelist\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_01e980d4",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_affiliateSplit\",\"type\":\"uint256\"}],\"name\":\"set_new_affiliateSplit\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_550bfa56",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_new_goodwill\",\"type\":\"uint256\"}],\"name\":\"set_new_goodwill\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_1385d24c",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"toggleContractActive\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_f2fde38b",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe80a9a878c4bed81f3803de10beff08ca8cd8c61_5ecb16cd",
        "{\"name\":\"Mushrooms Add\",\"address\":\"0xe80a9A878c4bED81f3803DE10beFF08Ca8Cd8c61\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"tokens\",\"type\":\"address[]\"}],\"name\":\"withdrawTokens\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe98984ad858075813ada4261af47e68a64e28fcc_c6ed8990",
        "{\"name\":\"Vested Escrow\",\"address\":\"0xe98984aD858075813AdA4261aF47e68A64E28fCC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"addTokens\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe98984ad858075813ada4261af47e68a64e28fcc_1e83409a",
        "{\"name\":\"Vested Escrow\",\"address\":\"0xe98984aD858075813AdA4261aF47e68A64E28fCC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_recipient\",\"type\":\"address\"}],\"name\":\"claim\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe98984ad858075813ada4261af47e68a64e28fcc_4e71d92d",
        "{\"name\":\"Vested Escrow\",\"address\":\"0xe98984aD858075813AdA4261aF47e68A64E28fCC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"claim\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe98984ad858075813ada4261af47e68a64e28fcc_cb1a4fc0",
        "{\"name\":\"Vested Escrow\",\"address\":\"0xe98984aD858075813AdA4261aF47e68A64E28fCC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"claimAndStake\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe98984ad858075813ada4261af47e68a64e28fcc_b1e56f6b",
        "{\"name\":\"Vested Escrow\",\"address\":\"0xe98984aD858075813AdA4261aF47e68A64E28fCC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"_recipient\",\"type\":\"address[]\"},{\"internalType\":\"uint256[]\",\"name\":\"_amount\",\"type\":\"uint256[]\"}],\"name\":\"fund\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe98984ad858075813ada4261af47e68a64e28fcc_704b6c02",
        "{\"name\":\"Vested Escrow\",\"address\":\"0xe98984aD858075813AdA4261aF47e68A64E28fCC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_admin\",\"type\":\"address\"}],\"name\":\"setAdmin\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe98984ad858075813ada4261af47e68a64e28fcc_a96d1edc",
        "{\"name\":\"Vested Escrow\",\"address\":\"0xe98984aD858075813AdA4261aF47e68A64E28fCC\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_fundadmin\",\"type\":\"address\"}],\"name\":\"setFundAdmin\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xedccb35798fae4925718a43cc608ae136208aa8d_58cbfd45",
        "{\"name\":\"Reward Factory\",\"address\":\"0xEdCCB35798fae4925718A43cc608aE136208aa8D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_depositToken\",\"type\":\"address\"}],\"name\":\"CreateCrvRewards\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xedccb35798fae4925718a43cc608ae136208aa8d_f8d6122e",
        "{\"name\":\"Reward Factory\",\"address\":\"0xEdCCB35798fae4925718A43cc608aE136208aa8D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_mainRewards\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_operator\",\"type\":\"address\"}],\"name\":\"CreateTokenRewards\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xedccb35798fae4925718a43cc608ae136208aa8d_b7f927b1",
        "{\"name\":\"Reward Factory\",\"address\":\"0xEdCCB35798fae4925718A43cc608aE136208aa8D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_reward\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"addActiveReward\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xedccb35798fae4925718a43cc608ae136208aa8d_ef9126ad",
        "{\"name\":\"Reward Factory\",\"address\":\"0xEdCCB35798fae4925718A43cc608aE136208aa8D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_reward\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"removeActiveReward\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xedccb35798fae4925718a43cc608ae136208aa8d_b84614a5",
        "{\"name\":\"Reward Factory\",\"address\":\"0xEdCCB35798fae4925718A43cc608aE136208aa8D\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_stash\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_status\",\"type\":\"bool\"}],\"name\":\"setAccess\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_7e29d6c2",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_lptoken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_stashVersion\",\"type\":\"uint256\"}],\"name\":\"addPool\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_6c7b69cb",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_gauge\",\"type\":\"address\"}],\"name\":\"claimRewards\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_43a0d066",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"_stake\",\"type\":\"bool\"}],\"name\":\"deposit\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_60759fce",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"bool\",\"name\":\"_stake\",\"type\":\"bool\"}],\"name\":\"depositAll\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_22230b96",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"earmarkFees\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_cc956f3f",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"earmarkRewards\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_71192b17",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_address\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"rewardClaimed\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_b0eefabe",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_arb\",\"type\":\"address\"}],\"name\":\"setArbitrator\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_7bd3b995",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_rfactory\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_sfactory\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_tfactory\",\"type\":\"address\"}],\"name\":\"setFactories\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_5a4ae5ca",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"setFeeInfo\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_472d35b9",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_feeM\",\"type\":\"address\"}],\"name\":\"setFeeManager\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_6fcba377",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_lockFees\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_stakerFees\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_callerFees\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_platform\",\"type\":\"uint256\"}],\"name\":\"setFees\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_9123d404",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"setGaugeRedirect\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_13af4035",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_owner\",\"type\":\"address\"}],\"name\":\"setOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_7aef6715",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_poolM\",\"type\":\"address\"}],\"name\":\"setPoolManager\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_95539a1d",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_rewards\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_stakerRewards\",\"type\":\"address\"}],\"name\":\"setRewardContracts\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_f0f44260",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_treasury\",\"type\":\"address\"}],\"name\":\"setTreasury\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_74874323",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_voteDelegate\",\"type\":\"address\"}],\"name\":\"setVoteDelegate\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_60cafe84",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"shutdownPool\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_354af919",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"shutdownSystem\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_e2cdd42a",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_voteId\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_votingAddress\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_support\",\"type\":\"bool\"}],\"name\":\"vote\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_bfad96ba",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"_gauge\",\"type\":\"address[]\"},{\"internalType\":\"uint256[]\",\"name\":\"_weight\",\"type\":\"uint256[]\"}],\"name\":\"voteGaugeWeight\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_441a3e70",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_958e2d31",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"}],\"name\":\"withdrawAll\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf403c135812408bfbe8713b5a23a04b3d48aae31_14cd70e4",
        "{\"name\":\"Booster\",\"address\":\"0xF403C135812408BFbE8713b5A23a04b3D48AAE31\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_pid\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_amount\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"_to\",\"type\":\"address\"}],\"name\":\"withdrawTo\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_d2423b51",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"bytes[]\",\"name\":\"calls\",\"type\":\"bytes[]\"},{\"internalType\":\"bool\",\"name\":\"revertOnFail\",\"type\":\"bool\"}],\"name\":\"batch\",\"outputs\":[{\"internalType\":\"bool[]\",\"name\":\"successes\",\"type\":\"bool[]\"},{\"internalType\":\"bytes[]\",\"name\":\"results\",\"type\":\"bytes[]\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_f483b3da",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IBatchFlashBorrower\",\"name\":\"borrower\",\"type\":\"address\"},{\"internalType\":\"address[]\",\"name\":\"receivers\",\"type\":\"address[]\"},{\"internalType\":\"contract IERC20[]\",\"name\":\"tokens\",\"type\":\"address[]\"},{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"batchFlashLoan\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_4e71e0c8",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"claimOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_1f54245b",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"masterContract\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"bool\",\"name\":\"useCreate2\",\"type\":\"bool\"}],\"name\":\"deploy\",\"outputs\":[{\"internalType\":\"address\",\"name\":\"cloneAddress\",\"type\":\"address\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_02b9446c",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token_\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"name\":\"deposit\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"shareOut\",\"type\":\"uint256\"}],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_f1676d37",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IFlashBorrower\",\"name\":\"borrower\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"receiver\",\"type\":\"address\"},{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"flashLoan\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_66c6bb0b",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"balance\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"maxChangeAmount\",\"type\":\"uint256\"}],\"name\":\"harvest\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_7c516e94",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"deadline\",\"type\":\"uint256\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"permitToken\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_aee4d1b2",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"registerProtocol\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_c0a47c93",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"user\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"masterContract\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"approved\",\"type\":\"bool\"},{\"internalType\":\"uint8\",\"name\":\"v\",\"type\":\"uint8\"},{\"internalType\":\"bytes32\",\"name\":\"r\",\"type\":\"bytes32\"},{\"internalType\":\"bytes32\",\"name\":\"s\",\"type\":\"bytes32\"}],\"name\":\"setMasterContractApproval\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_72cb5d97",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"contract IStrategy\",\"name\":\"newStrategy\",\"type\":\"address\"}],\"name\":\"setStrategy\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_3e2a9d4e",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"uint64\",\"name\":\"targetPercentage_\",\"type\":\"uint64\"}],\"name\":\"setStrategyTargetPercentage\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_f18d03cc",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_0fca8843",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address[]\",\"name\":\"tos\",\"type\":\"address[]\"},{\"internalType\":\"uint256[]\",\"name\":\"shares\",\"type\":\"uint256[]\"}],\"name\":\"transferMultiple\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_078dfbe7",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"direct\",\"type\":\"bool\"},{\"internalType\":\"bool\",\"name\":\"renounce\",\"type\":\"bool\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_733a9d7c",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"masterContract\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"approved\",\"type\":\"bool\"}],\"name\":\"whitelistMasterContract\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xf5bce5077908a1b7370b9ae04adc565ebd643966_97da6d30",
        "{\"name\":\"BentoBoxV1\",\"address\":\"0xF5BCE5077908a1b7370B9ae04AdC565EBd643966\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"contract IERC20\",\"name\":\"token_\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"share\",\"type\":\"uint256\"}],\"name\":\"withdraw\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"amountOut\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"shareOut\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaa67e3736572645b38af7410b3e1006708e13f4_715018a6",
        "{\"name\":\"MarginCalculator\",\"address\":\"0xfaa67e3736572645B38AF7410B3E1006708e13F4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaa67e3736572645b38af7410b3e1006708e13f4_9a783d11",
        "{\"name\":\"MarginCalculator\",\"address\":\"0xfaa67e3736572645B38AF7410B3E1006708e13F4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_dust\",\"type\":\"uint256\"}],\"name\":\"setCollateralDust\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaa67e3736572645b38af7410b3e1006708e13f4_c73547e3",
        "{\"name\":\"MarginCalculator\",\"address\":\"0xfaa67e3736572645B38AF7410B3E1006708e13F4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_deviation\",\"type\":\"uint256\"}],\"name\":\"setOracleDeviation\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaa67e3736572645b38af7410b3e1006708e13f4_8aab8fa5",
        "{\"name\":\"MarginCalculator\",\"address\":\"0xfaa67e3736572645B38AF7410B3E1006708e13F4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_underlying\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_strike\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_isPut\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"_shockValue\",\"type\":\"uint256\"}],\"name\":\"setSpotShock\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaa67e3736572645b38af7410b3e1006708e13f4_7f519b72",
        "{\"name\":\"MarginCalculator\",\"address\":\"0xfaa67e3736572645B38AF7410B3E1006708e13F4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_underlying\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_strike\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_isPut\",\"type\":\"bool\"},{\"internalType\":\"uint256[]\",\"name\":\"_timesToExpiry\",\"type\":\"uint256[]\"},{\"internalType\":\"uint256[]\",\"name\":\"_values\",\"type\":\"uint256[]\"}],\"name\":\"setUpperBoundValues\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaa67e3736572645b38af7410b3e1006708e13f4_f2fde38b",
        "{\"name\":\"MarginCalculator\",\"address\":\"0xfaa67e3736572645B38AF7410B3E1006708e13F4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaa67e3736572645b38af7410b3e1006708e13f4_7ad7f800",
        "{\"name\":\"MarginCalculator\",\"address\":\"0xfaa67e3736572645B38AF7410B3E1006708e13F4\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_underlying\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_strike\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"_collateral\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"_isPut\",\"type\":\"bool\"},{\"internalType\":\"uint256\",\"name\":\"_timeToExpiry\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"updateUpperBoundValue\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_93eea246",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_account\",\"type\":\"address\"},{\"internalType\":\"uint256[]\",\"name\":\"_ids\",\"type\":\"uint256[]\"},{\"internalType\":\"bytes[]\",\"name\":\"_signatures\",\"type\":\"bytes[]\"},{\"internalType\":\"bytes[]\",\"name\":\"_data\",\"type\":\"bytes[]\"}],\"name\":\"batchMint\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_bcebbe80",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"string\",\"name\":\"_cid\",\"type\":\"string\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"create\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"_id\",\"type\":\"uint256\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_dcdc7dd0",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_account\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_id\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"_signature\",\"type\":\"bytes\"},{\"internalType\":\"bytes\",\"name\":\"_data\",\"type\":\"bytes\"}],\"name\":\"mint\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_8456cb59",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"pause\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_715018a6",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"name\":\"renounceOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_2eb2c2d6",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256[]\",\"name\":\"ids\",\"type\":\"uint256[]\"},{\"internalType\":\"uint256[]\",\"name\":\"amounts\",\"type\":\"uint256[]\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"safeBatchTransferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_f242432a",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"from\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"id\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"}],\"name\":\"safeTransferFrom\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_a22cb465",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"operator\",\"type\":\"address\"},{\"internalType\":\"bool\",\"name\":\"approved\",\"type\":\"bool\"}],\"name\":\"setApprovalForAll\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_f2fde38b",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"transferOwnership\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xfaff15c6cdaca61a4f87d329689293e07c98f578_a7ecd37e",
        "{\"name\":\"Zapper NFT\",\"address\":\"0xFAFf15C6cDAca61a4F87D329689293E07c98f578\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"signer\",\"type\":\"address\"}],\"name\":\"updateSigner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e1dcf7ad4e460cfd30791ccc4f9c8a4f820ec67_ec9e80bb",
        "{\"name\":\"Safe Proxy Factory 1.4.1\",\"address\":\"0x4e1dcf7ad4e460cfd30791ccc4f9c8a4f820ec67\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_singleton\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"initializer\",\"type\":\"bytes\"},{\"internalType\":\"uint256\",\"name\":\"saltNonce\",\"type\":\"uint256\"}],\"name\":\"createChainSpecificProxyWithNonce\",\"outputs\":[{\"internalType\":\"contract SafeProxy\",\"name\":\"proxy\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e1dcf7ad4e460cfd30791ccc4f9c8a4f820ec67_d18af54d",
        "{\"name\":\"Safe Proxy Factory 1.4.1\",\"address\":\"0x4e1dcf7ad4e460cfd30791ccc4f9c8a4f820ec67\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_singleton\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"initializer\",\"type\":\"bytes\"},{\"internalType\":\"uint256\",\"name\":\"saltNonce\",\"type\":\"uint256\"},{\"internalType\":\"contract IProxyCreationCallback\",\"name\":\"callback\",\"type\":\"address\"}],\"name\":\"createProxyWithCallback\",\"outputs\":[{\"internalType\":\"contract SafeProxy\",\"name\":\"proxy\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0x4e1dcf7ad4e460cfd30791ccc4f9c8a4f820ec67_1688f0b9",
        "{\"name\":\"Safe Proxy Factory 1.4.1\",\"address\":\"0x4e1dcf7ad4e460cfd30791ccc4f9c8a4f820ec67\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address\",\"name\":\"_singleton\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"initializer\",\"type\":\"bytes\"},{\"internalType\":\"uint256\",\"name\":\"saltNonce\",\"type\":\"uint256\"}],\"name\":\"createProxyWithNonce\",\"outputs\":[{\"internalType\":\"contract SafeProxy\",\"name\":\"proxy\",\"type\":\"address\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146_1fece7b4",
        "{\"name\":\"THORChain Router\",\"address\":\"0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"vault\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"string\",\"name\":\"memo\",\"type\":\"string\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146_44bc937b",
        "{\"name\":\"THORChain Router\",\"address\":\"0xd37bbe5744d730a1d98d8dc97c42f0ca46ad7146\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"vault\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"string\",\"name\":\"memo\",\"type\":\"string\"},{\"internalType\":\"uint256\",\"name\":\"expiration\",\"type\":\"uint256\"}],\"name\":\"depositWithExpiry\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe3985E6b61b814F7Cdb188766562ba71b446B46d_1fece7b4",
        "{\"name\":\"MAYAChain Router\",\"address\":\"0xe3985E6b61b814F7Cdb188766562ba71b446B46d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"vault\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"string\",\"name\":\"memo\",\"type\":\"string\"}],\"name\":\"deposit\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    },
    {
        "0xe3985E6b61b814F7Cdb188766562ba71b446B46d_44bc937b",
        "{\"name\":\"MAYAChain Router\",\"address\":\"0xe3985E6b61b814F7Cdb188766562ba71b446B46d\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[{\"internalType\":\"address payable\",\"name\":\"vault\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"asset\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"amount\",\"type\":\"uint256\"},{\"internalType\":\"string\",\"name\":\"memo\",\"type\":\"string\"},{\"internalType\":\"uint256\",\"name\":\"expiration\",\"type\":\"uint256\"}],\"name\":\"depositWithExpiry\",\"outputs\":[],\"stateMutability\":\"payable\",\"type\":\"function\"}]}},\"version\":1,\"checkPoints\":[]}"
    }
};

uint32_t GetEthereumABIMapSize()
{
    return NUMBER_OF_ARRAYS(ethereum_abi_map);
}

const char *ethereum_erc20_json = "{\"name\":\"Erc20\",\"address\":\"\",\"metadata\":{\"output\":{\"abi\":[{\"constant\":true,\"inputs\":[],\"name\":\"name\",\"outputs\":[{\"name\":\"\",\"type\":\"string\"}],\"payable\":false,\"stateMutability\":\"view\",\"type\":\"function\"},{\"constant\":false,\"inputs\":[{\"name\":\"_spender\",\"type\":\"address\"},{\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"approve\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"constant\":true,\"inputs\":[],\"name\":\"totalSupply\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"view\",\"type\":\"function\"},{\"constant\":false,\"inputs\":[{\"name\":\"_from\",\"type\":\"address\"},{\"name\":\"_to\",\"type\":\"address\"},{\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"transferFrom\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"constant\":true,\"inputs\":[],\"name\":\"decimals\",\"outputs\":[{\"name\":\"\",\"type\":\"uint8\"}],\"payable\":false,\"stateMutability\":\"view\",\"type\":\"function\"},{\"constant\":true,\"inputs\":[{\"name\":\"_owner\",\"type\":\"address\"}],\"name\":\"balanceOf\",\"outputs\":[{\"name\":\"balance\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"view\",\"type\":\"function\"},{\"constant\":true,\"inputs\":[],\"name\":\"symbol\",\"outputs\":[{\"name\":\"\",\"type\":\"string\"}],\"payable\":false,\"stateMutability\":\"view\",\"type\":\"function\"},{\"constant\":false,\"inputs\":[{\"name\":\"_to\",\"type\":\"address\"},{\"name\":\"_value\",\"type\":\"uint256\"}],\"name\":\"transfer\",\"outputs\":[{\"name\":\"\",\"type\":\"bool\"}],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"constant\":true,\"inputs\":[{\"name\":\"_owner\",\"type\":\"address\"},{\"name\":\"_spender\",\"type\":\"address\"}],\"name\":\"allowance\",\"outputs\":[{\"name\":\"\",\"type\":\"uint256\"}],\"payable\":false,\"stateMutability\":\"view\",\"type\":\"function\"},{\"payable\":true,\"stateMutability\":\"payable\",\"type\":\"fallback\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"name\":\"owner\",\"type\":\"address\"},{\"indexed\":true,\"name\":\"spender\",\"type\":\"address\"},{\"indexed\":false,\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"Approval\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"name\":\"from\",\"type\":\"address\"},{\"indexed\":true,\"name\":\"to\",\"type\":\"address\"},{\"indexed\":false,\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"Transfer\",\"type\":\"event\"}]}},\"version\":1,\"checkPoints\":[]}";

const char *safe_json = "{\"name\":\"Safe Contract Wallet\",\"address\":\"\",\"metadata\":{\"output\":{\"abi\":[{\"inputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"constructor\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"}],\"name\":\"AddedOwner\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"bytes32\",\"name\":\"approvedHash\",\"type\":\"bytes32\"},{\"indexed\":true,\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"}],\"name\":\"ApproveHash\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"handler\",\"type\":\"address\"}],\"name\":\"ChangedFallbackHandler\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"guard\",\"type\":\"address\"}],\"name\":\"ChangedGuard\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":false,\"internalType\":\"uint256\",\"name\":\"threshold\",\"type\":\"uint256\"}],\"name\":\"ChangedThreshold\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"DisabledModule\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"EnabledModule\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"bytes32\",\"name\":\"txHash\",\"type\":\"bytes32\"},{\"indexed\":false,\"internalType\":\"uint256\",\"name\":\"payment\",\"type\":\"uint256\"}],\"name\":\"ExecutionFailure\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"ExecutionFromModuleFailure\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"ExecutionFromModuleSuccess\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"bytes32\",\"name\":\"txHash\",\"type\":\"bytes32\"},{\"indexed\":false,\"internalType\":\"uint256\",\"name\":\"payment\",\"type\":\"uint256\"}],\"name\":\"ExecutionSuccess\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"}],\"name\":\"RemovedOwner\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"sender\",\"type\":\"address\"},{\"indexed\":false,\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"}],\"name\":\"SafeReceived\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"address\",\"name\":\"initiator\",\"type\":\"address\"},{\"indexed\":false,\"internalType\":\"address[]\",\"name\":\"owners\",\"type\":\"address[]\"},{\"indexed\":false,\"internalType\":\"uint256\",\"name\":\"threshold\",\"type\":\"uint256\"},{\"indexed\":false,\"internalType\":\"address\",\"name\":\"initializer\",\"type\":\"address\"},{\"indexed\":false,\"internalType\":\"address\",\"name\":\"fallbackHandler\",\"type\":\"address\"}],\"name\":\"SafeSetup\",\"type\":\"event\"},{\"anonymous\":false,\"inputs\":[{\"indexed\":true,\"internalType\":\"bytes32\",\"name\":\"msgHash\",\"type\":\"bytes32\"}],\"name\":\"SignMsg\",\"type\":\"event\"},{\"stateMutability\":\"nonpayable\",\"type\":\"fallback\"},{\"inputs\":[],\"name\":\"VERSION\",\"outputs\":[{\"internalType\":\"string\",\"name\":\"\",\"type\":\"string\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"addOwnerWithThreshold\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"hashToApprove\",\"type\":\"bytes32\"}],\"name\":\"approveHash\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"\",\"type\":\"address\"},{\"internalType\":\"bytes32\",\"name\":\"\",\"type\":\"bytes32\"}],\"name\":\"approvedHashes\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"changeThreshold\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"dataHash\",\"type\":\"bytes32\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"bytes\",\"name\":\"signatures\",\"type\":\"bytes\"},{\"internalType\":\"uint256\",\"name\":\"requiredSignatures\",\"type\":\"uint256\"}],\"name\":\"checkNSignatures\",\"outputs\":[],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"dataHash\",\"type\":\"bytes32\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"bytes\",\"name\":\"signatures\",\"type\":\"bytes\"}],\"name\":\"checkSignatures\",\"outputs\":[],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevModule\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"disableModule\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[],\"name\":\"domainSeparator\",\"outputs\":[{\"internalType\":\"bytes32\",\"name\":\"\",\"type\":\"bytes32\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"enableModule\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"},{\"internalType\":\"uint256\",\"name\":\"safeTxGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"baseGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"gasPrice\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"gasToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"refundReceiver\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_nonce\",\"type\":\"uint256\"}],\"name\":\"encodeTransactionData\",\"outputs\":[{\"internalType\":\"bytes\",\"name\":\"\",\"type\":\"bytes\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"},{\"internalType\":\"uint256\",\"name\":\"safeTxGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"baseGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"gasPrice\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"gasToken\",\"type\":\"address\"},{\"internalType\":\"address payable\",\"name\":\"refundReceiver\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"signatures\",\"type\":\"bytes\"}],\"name\":\"execTransaction\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"}],\"stateMutability\":\"payable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"execTransactionFromModule\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"}],\"name\":\"execTransactionFromModuleReturnData\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"success\",\"type\":\"bool\"},{\"internalType\":\"bytes\",\"name\":\"returnData\",\"type\":\"bytes\"}],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[],\"name\":\"getChainId\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"start\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"pageSize\",\"type\":\"uint256\"}],\"name\":\"getModulesPaginated\",\"outputs\":[{\"internalType\":\"address[]\",\"name\":\"array\",\"type\":\"address[]\"},{\"internalType\":\"address\",\"name\":\"next\",\"type\":\"address\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[],\"name\":\"getOwners\",\"outputs\":[{\"internalType\":\"address[]\",\"name\":\"\",\"type\":\"address[]\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"uint256\",\"name\":\"offset\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"length\",\"type\":\"uint256\"}],\"name\":\"getStorageAt\",\"outputs\":[{\"internalType\":\"bytes\",\"name\":\"\",\"type\":\"bytes\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[],\"name\":\"getThreshold\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"value\",\"type\":\"uint256\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"enum Enum.Operation\",\"name\":\"operation\",\"type\":\"uint8\"},{\"internalType\":\"uint256\",\"name\":\"safeTxGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"baseGas\",\"type\":\"uint256\"},{\"internalType\":\"uint256\",\"name\":\"gasPrice\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"gasToken\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"refundReceiver\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_nonce\",\"type\":\"uint256\"}],\"name\":\"getTransactionHash\",\"outputs\":[{\"internalType\":\"bytes32\",\"name\":\"\",\"type\":\"bytes32\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"module\",\"type\":\"address\"}],\"name\":\"isModuleEnabled\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"}],\"name\":\"isOwner\",\"outputs\":[{\"internalType\":\"bool\",\"name\":\"\",\"type\":\"bool\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[],\"name\":\"nonce\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"owner\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"}],\"name\":\"removeOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"handler\",\"type\":\"address\"}],\"name\":\"setFallbackHandler\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"guard\",\"type\":\"address\"}],\"name\":\"setGuard\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address[]\",\"name\":\"_owners\",\"type\":\"address[]\"},{\"internalType\":\"uint256\",\"name\":\"_threshold\",\"type\":\"uint256\"},{\"internalType\":\"address\",\"name\":\"to\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"data\",\"type\":\"bytes\"},{\"internalType\":\"address\",\"name\":\"fallbackHandler\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"paymentToken\",\"type\":\"address\"},{\"internalType\":\"uint256\",\"name\":\"payment\",\"type\":\"uint256\"},{\"internalType\":\"address payable\",\"name\":\"paymentReceiver\",\"type\":\"address\"}],\"name\":\"setup\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"bytes32\",\"name\":\"\",\"type\":\"bytes32\"}],\"name\":\"signedMessages\",\"outputs\":[{\"internalType\":\"uint256\",\"name\":\"\",\"type\":\"uint256\"}],\"stateMutability\":\"view\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"targetContract\",\"type\":\"address\"},{\"internalType\":\"bytes\",\"name\":\"calldataPayload\",\"type\":\"bytes\"}],\"name\":\"simulateAndRevert\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"inputs\":[{\"internalType\":\"address\",\"name\":\"prevOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"oldOwner\",\"type\":\"address\"},{\"internalType\":\"address\",\"name\":\"newOwner\",\"type\":\"address\"}],\"name\":\"swapOwner\",\"outputs\":[],\"stateMutability\":\"nonpayable\",\"type\":\"function\"},{\"stateMutability\":\"payable\",\"type\":\"receive\"}]}},\"version\":1,\"checkPoints\":[]}";
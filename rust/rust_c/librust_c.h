#ifndef _LIBRUST_C_H
#define _LIBRUST_C_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#define BUILD_MULTI_COINS
#define SIMPLERESPONSE_C_CHAR_MAX_LEN 2048
#define ADDRESS_MAX_LEN 256
#define PATH_ITEM_MAX_LEN 32

typedef enum ArweaveRequestType {
  ArweaveRequestTypeTransaction = 1,
  ArweaveRequestTypeDataItem,
  ArweaveRequestTypeMessage,
  ArweaveRequestTypeUnknown,
} ArweaveRequestType;

#if defined(BUILD_MULTI_COINS)
typedef enum ETHAccountType {
#if defined(BUILD_MULTI_COINS)
  Bip44Standard,
#endif
#if defined(BUILD_MULTI_COINS)
  LedgerLive,
#endif
#if defined(BUILD_MULTI_COINS)
  LedgerLegacy,
#endif
} ETHAccountType;
#endif

typedef enum ErrorCodes {
  Success = 0,
  InvalidMasterFingerprint,
  MasterFingerprintMismatch,
  InvalidHDPath,
  UnsupportedTransaction,
  InvalidXPub,
  SignFailure,
  UnexpectedError,
  InvalidHex,
  WebAuthFailed,
  InvalidData,
  CborDecodeError = 20,
  CborEncodeError,
  URDecodeError,
  UREncodeError,
  NotSupportURTypeError,
  NotAnUr,
  URTypeUnspecified,
  URProtobufDecodeError,
  URProtobufEncodeError,
  URGzipDecodeError,
  URGzipEnCodeError,
  KeystoreSeedError = 40,
  KeystoreDerivationError,
  KeystoreXPubError,
  KeystoreInvalidDerivationPath,
  KeystoreDeivePubkey,
  KeystoreGenerateSigningKeyError,
  KeystoreRSASignError,
  KeystoreRSAVerifyError,
  KeystoreInvalidDataError,
  KeystoreZcashOrchardSignError,
  BitcoinInvalidInput = 100,
  BitcoinInvalidOutput,
  BitcoinInvalidPsbt,
  BitcoinInvalidTransaction,
  BitcoinNoInputs,
  BitcoinNoOutputs,
  BitcoinNoMyInputs,
  BitcoinInputValueTampered,
  BitcoinSignFailure,
  BitcoinAddressError,
  BitcoinGetKeyError,
  BitcoinSignLegacyTxError,
  BitcoinUnsupportedTransaction,
  BitcoinUnsupportedNetwork,
  BitcoinTransactionConsensusEncodeError,
  BitcoinPushBytesFailed,
  BitcoinInvalidHex,
  BitcoinBase58Error,
  BitcoinKeystoreError,
  BitcoinInvalidParseContext,
  BitcoinInvalidRawTxCryptoBytes,
  BitcoinInvalidTxData,
  BitcoinUnsupportedScriptType,
  BitcoinBech32DecodeError,
  BitcoinWitnessProgramError,
  BitcoinMultiSigWalletParseError,
  BitcoinMultiSigWalletNotMyWallet,
  BitcoinMultiSigWalletAddressCalError,
  BitcoinMultiSigWalletImportXpubError,
  BitcoinMultiSigWalletCrateError,
  BitcoinMultiSigWalletFormatError,
  BitcoinMultiSigNetworkError,
  BitcoinMultiSigInputError,
  BitcoinDerivePublicKeyError,
  BitcoinWalletTypeError,
  EthereumRlpDecodingError = 200,
  EthereumInvalidTransaction,
  EthereumSignFailure,
  EthereumInvalidHDPath,
  EthereumKeystoreError,
  EthereumInvalidAddressError,
  EthereumInvalidUtf8Error,
  EthereumInvalidContractABI,
  EthereumDecodeContractDataError,
  EthereumInvalidTypedData,
  EthereumHashTypedDataError,
  TronInvalidRawTxCryptoBytes = 300,
  TronInvalidParseContext,
  TronBase58Error,
  TronSignFailure,
  TronProtobufError,
  TronParseNumberError,
  TronNoMyInputs,
  CompanionAppProtobufError = 400,
  CompanionAppInvalidParseContext,
  CompanionAppSignTxFailed,
  CompanionAppCheckTxFailed,
  CardanoInvalidTransaction = 500,
  CardanoAddressEncodingError,
  SolanaAddressEncodingError = 600,
  SolanaKeystoreError,
  SolanaUnsupportedProgram,
  SolanaInvalidData,
  SolanaAccountNotFound,
  SolanaProgramError,
  SolanaParseTxError,
  NearKeystoreError = 700,
  NearSignFailure,
  NearParseTxError,
  XRPSignFailure = 800,
  XRPInvalidData,
  XRPParseTxError,
  CosmosSignFailure = 900,
  CosmosKeystoreError,
  CosmosInvalidData,
  CosmosParseTxError,
  AptosSignFailure = 1000,
  AptosKeystoreError,
  AptosInvalidData,
  AptosParseTxError,
  SuiSignFailure = 1100,
  SuiInvalidData,
  SuiParseTxError,
  ArweaveSignFailure = 1200,
  ArweaveKeystoreError,
  ArweaveInvalidData,
  ArweaveParseTxError,
  ArweaveParseAOTxError,
  TonUnknownError = 1300,
  TonMnemonicError,
  TonTransactionError,
  InvalidProof,
  TonTransactionJsonError,
  AddressError,
  StellarAddressError = 1400,
  StellarInvalidData,
  StellarParseTxError,
  StellarKeystoreError,
  ZcashGenerateAddressError = 1500,
  ZcashSigningError,
  ZcashInvalidPczt,
} ErrorCodes;

typedef enum NetworkType {
  MainNet,
  TestNet,
} NetworkType;

typedef enum QRCodeType {
  CryptoPSBT,
  CryptoMultiAccounts,
  CryptoAccount,
  Bytes,
  BtcSignRequest,
  SeedSignerMessage,
#if defined(BUILD_MULTI_COINS)
  KeystoneSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  EthSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  SolSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  NearSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoSignTxHashRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoSignDataRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoCatalystVotingRegistrationRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  CosmosSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  EvmSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  SuiSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  SuiSignHashRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  AptosSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  QRHardwareCall,
#endif
#if defined(BUILD_MULTI_COINS)
  ArweaveSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  StellarSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  TonSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  ZcashPczt,
#endif
  URTypeUnKnown,
} QRCodeType;

typedef enum QRProtocol {
  QRCodeTypeText,
  QRCodeTypeUR,
} QRProtocol;

typedef enum ViewType {
  BtcNativeSegwitTx,
  BtcSegwitTx,
  BtcLegacyTx,
  BtcTx,
  BtcMsg,
#if defined(BUILD_MULTI_COINS)
  LtcTx,
#endif
#if defined(BUILD_MULTI_COINS)
  DashTx,
#endif
#if defined(BUILD_MULTI_COINS)
  BchTx,
#endif
#if defined(BUILD_MULTI_COINS)
  EthTx,
#endif
#if defined(BUILD_MULTI_COINS)
  EthPersonalMessage,
#endif
#if defined(BUILD_MULTI_COINS)
  EthTypedData,
#endif
#if defined(BUILD_MULTI_COINS)
  TronTx,
#endif
#if defined(BUILD_MULTI_COINS)
  SolanaTx,
#endif
#if defined(BUILD_MULTI_COINS)
  SolanaMessage,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoTx,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoSignData,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoCatalystVotingRegistration,
#endif
#if defined(BUILD_MULTI_COINS)
  CardanoSignTxHash,
#endif
#if defined(BUILD_MULTI_COINS)
  NearTx,
#endif
#if defined(BUILD_MULTI_COINS)
  XRPTx,
#endif
#if defined(BUILD_MULTI_COINS)
  CosmosTx,
#endif
#if defined(BUILD_MULTI_COINS)
  CosmosEvmTx,
#endif
#if defined(BUILD_MULTI_COINS)
  SuiTx,
#endif
#if defined(BUILD_MULTI_COINS)
  SuiSignMessageHash,
#endif
#if defined(BUILD_MULTI_COINS)
  ArweaveTx,
#endif
#if defined(BUILD_MULTI_COINS)
  ArweaveMessage,
#endif
#if defined(BUILD_MULTI_COINS)
  ArweaveDataItem,
#endif
#if defined(BUILD_MULTI_COINS)
  StellarTx,
#endif
#if defined(BUILD_MULTI_COINS)
  StellarHash,
#endif
#if defined(BUILD_MULTI_COINS)
  TonTx,
#endif
#if defined(BUILD_MULTI_COINS)
  TonSignProof,
#endif
#if defined(BUILD_MULTI_COINS)
  ZcashTx,
#endif
#if defined(BUILD_MULTI_COINS)
  AptosTx,
#endif
  WebAuthResult,
#if defined(BUILD_MULTI_COINS)
  KeyDerivationRequest,
#endif
  ViewTypeUnKnown,
} ViewType;

typedef char *PtrString;

typedef struct SimpleResponse_u8 {
  uint8_t *data;
  uint32_t error_code;
  PtrString error_message;
} SimpleResponse_u8;

typedef uint8_t *PtrBytes;

typedef struct SimpleResponse_c_char {
  char *data;
  uint32_t error_code;
  PtrString error_message;
} SimpleResponse_c_char;

typedef struct TransactionCheckResult {
  uint32_t error_code;
  PtrString error_message;
} TransactionCheckResult;

typedef struct TransactionCheckResult *Ptr_TransactionCheckResult;

typedef void *PtrVoid;

typedef PtrVoid PtrUR;

typedef PtrVoid PtrDecoder;

typedef struct URParseResult {
  bool is_multi_part;
  uint32_t progress;
  enum ViewType t;
  enum QRCodeType ur_type;
  PtrUR data;
  PtrDecoder decoder;
  uint32_t error_code;
  PtrString error_message;
} URParseResult;

typedef struct URParseResult *PtrT_URParseResult;

typedef struct URParseMultiResult {
  bool is_complete;
  enum ViewType t;
  enum QRCodeType ur_type;
  uint32_t progress;
  PtrUR data;
  uint32_t error_code;
  PtrString error_message;
} URParseMultiResult;

typedef struct URParseMultiResult *PtrT_URParseMultiResult;

typedef PtrVoid PtrEncoder;

typedef struct UREncodeResult {
  bool is_multi_part;
  char *data;
  PtrEncoder encoder;
  uint32_t error_code;
  char *error_message;
} UREncodeResult;

typedef struct UREncodeResult *PtrT_UREncodeResult;

typedef struct UREncodeMultiResult {
  char *data;
  uint32_t error_code;
  char *error_message;
} UREncodeMultiResult;

typedef struct UREncodeMultiResult *PtrT_UREncodeMultiResult;

typedef struct SimpleResponse_u8 *PtrT_SimpleResponse_u8;

typedef struct SimpleResponse_c_char *PtrT_SimpleResponse_c_char;

typedef uint8_t *PtrT_u8;

typedef struct VecFFI_u8 {
  PtrT_u8 data;
  size_t size;
  size_t cap;
} VecFFI_u8;

typedef struct VecFFI_u8 *PtrT_VecFFI_u8;

typedef struct URParseResult *Ptr_URParseResult;

typedef struct TransactionCheckResult *PtrT_TransactionCheckResult;

typedef const void *ConstPtrVoid;

typedef ConstPtrVoid ConstPtrUR;

typedef struct ExtendedPublicKey {
  PtrString path;
  PtrString xpub;
} ExtendedPublicKey;

typedef struct ExtendedPublicKey *PtrT_ExtendedPublicKey;

typedef struct CSliceFFI_ExtendedPublicKey {
  PtrT_ExtendedPublicKey data;
  size_t size;
} CSliceFFI_ExtendedPublicKey;

typedef struct CSliceFFI_ExtendedPublicKey *PtrT_CSliceFFI_ExtendedPublicKey;

typedef struct AccountConfig {
  PtrString hd_path;
  PtrString x_pub;
  int32_t address_length;
  bool is_multi_sign;
} AccountConfig;

typedef struct CoinConfig {
  bool is_active;
  PtrString coin_code;
  struct AccountConfig *accounts;
  uint32_t accounts_length;
} CoinConfig;

typedef struct CoinConfig *PtrT_CoinConfig;

typedef struct UREncodeResult *Ptr_UREncodeResult;

typedef struct CSliceFFI_ExtendedPublicKey *Ptr_CSliceFFI_ExtendedPublicKey;

#if defined(BUILD_MULTI_COINS)
typedef struct KeyDerivationSchema {
  PtrString key_path;
  PtrString curve;
  PtrString algo;
  PtrString chain_type;
  bool is_ada;
} KeyDerivationSchema;
#endif

typedef struct KeyDerivationSchema *PtrT_KeyDerivationSchema;

typedef struct VecFFI_KeyDerivationSchema {
  PtrT_KeyDerivationSchema data;
  size_t size;
  size_t cap;
} VecFFI_KeyDerivationSchema;

typedef struct VecFFI_KeyDerivationSchema *Ptr_VecFFI_KeyDerivationSchema;

#if defined(BUILD_MULTI_COINS)
typedef struct KeyDerivationRequestData {
  Ptr_VecFFI_KeyDerivationSchema schemas;
} KeyDerivationRequestData;
#endif

typedef struct KeyDerivationRequestData *Ptr_KeyDerivationRequestData;

#if defined(BUILD_MULTI_COINS)
typedef struct QRHardwareCallData {
  PtrString call_type;
  PtrString origin;
  Ptr_KeyDerivationRequestData key_derivation;
  PtrString version;
} QRHardwareCallData;
#endif

typedef struct Response_QRHardwareCallData {
  struct QRHardwareCallData *data;
  uint32_t error_code;
  PtrString error_message;
} Response_QRHardwareCallData;

typedef struct Response_QRHardwareCallData *Ptr_Response_QRHardwareCallData;

typedef struct Response_bool {
  bool *data;
  uint32_t error_code;
  PtrString error_message;
} Response_bool;

#if defined(BUILD_MULTI_COINS)
typedef struct KeplrAccount {
  PtrString name;
  PtrString path;
  PtrString xpub;
} KeplrAccount;
#endif

typedef struct KeplrAccount *PtrT_KeplrAccount;

typedef struct CSliceFFI_KeplrAccount {
  PtrT_KeplrAccount data;
  size_t size;
} CSliceFFI_KeplrAccount;

typedef struct CSliceFFI_KeplrAccount *PtrT_CSliceFFI_KeplrAccount;

typedef struct Response_QRHardwareCallData *PtrT_Response_QRHardwareCallData;

typedef struct ZcashKey {
  PtrString key_text;
  PtrString key_name;
  uint32_t index;
} ZcashKey;

typedef struct ZcashKey *PtrT_ZcashKey;

typedef struct CSliceFFI_ZcashKey {
  PtrT_ZcashKey data;
  size_t size;
} CSliceFFI_ZcashKey;

typedef struct CSliceFFI_ZcashKey *Ptr_CSliceFFI_ZcashKey;

typedef struct DisplayAptosTx {
  PtrString detail;
  bool is_msg;
} DisplayAptosTx;

typedef struct TransactionParseResult_DisplayAptosTx {
  struct DisplayAptosTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayAptosTx;

typedef struct TransactionParseResult_DisplayAptosTx *PtrT_TransactionParseResult_DisplayAptosTx;

typedef struct SimpleResponse_ArweaveRequestType {
  enum ArweaveRequestType *data;
  uint32_t error_code;
  PtrString error_message;
} SimpleResponse_ArweaveRequestType;

typedef struct DisplayArweaveMessage {
  PtrString message;
  PtrString raw_message;
} DisplayArweaveMessage;

typedef struct TransactionParseResult_DisplayArweaveMessage {
  struct DisplayArweaveMessage *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayArweaveMessage;

typedef struct TransactionParseResult_DisplayArweaveMessage *PtrT_TransactionParseResult_DisplayArweaveMessage;

typedef struct DisplayArweaveTx {
  PtrString value;
  PtrString fee;
  PtrString from;
  PtrString to;
  PtrString detail;
} DisplayArweaveTx;

typedef struct TransactionParseResult_DisplayArweaveTx {
  struct DisplayArweaveTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayArweaveTx;

typedef struct TransactionParseResult_DisplayArweaveTx *PtrT_TransactionParseResult_DisplayArweaveTx;

typedef struct DisplayTag {
  PtrString name;
  PtrString value;
} DisplayTag;

typedef struct DisplayTag *PtrT_DisplayTag;

typedef struct VecFFI_DisplayTag {
  PtrT_DisplayTag data;
  size_t size;
  size_t cap;
} VecFFI_DisplayTag;

typedef struct VecFFI_DisplayTag *Ptr_VecFFI_DisplayTag;

typedef struct DisplayArweaveDataItem {
  PtrString owner;
  PtrString target;
  PtrString anchor;
  Ptr_VecFFI_DisplayTag tags;
  PtrString data;
} DisplayArweaveDataItem;

typedef struct TransactionParseResult_DisplayArweaveDataItem {
  struct DisplayArweaveDataItem *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayArweaveDataItem;

typedef struct TransactionParseResult_DisplayArweaveDataItem *PtrT_TransactionParseResult_DisplayArweaveDataItem;

typedef struct DisplayArweaveAOTransfer {
  PtrString from;
  PtrString to;
  PtrString quantity;
  PtrString token_id;
  Ptr_VecFFI_DisplayTag other_info;
} DisplayArweaveAOTransfer;

typedef struct TransactionParseResult_DisplayArweaveAOTransfer {
  struct DisplayArweaveAOTransfer *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayArweaveAOTransfer;

typedef struct TransactionParseResult_DisplayArweaveAOTransfer *PtrT_TransactionParseResult_DisplayArweaveAOTransfer;

typedef struct DisplayTxOverviewInput {
  PtrString address;
} DisplayTxOverviewInput;

typedef struct DisplayTxOverviewInput *PtrT_DisplayTxOverviewInput;

typedef struct VecFFI_DisplayTxOverviewInput {
  PtrT_DisplayTxOverviewInput data;
  size_t size;
  size_t cap;
} VecFFI_DisplayTxOverviewInput;

typedef struct VecFFI_DisplayTxOverviewInput *PtrT_VecFFI_DisplayTxOverviewInput;

typedef struct DisplayTxOverviewOutput {
  PtrString address;
} DisplayTxOverviewOutput;

typedef struct DisplayTxOverviewOutput *PtrT_DisplayTxOverviewOutput;

typedef struct VecFFI_DisplayTxOverviewOutput {
  PtrT_DisplayTxOverviewOutput data;
  size_t size;
  size_t cap;
} VecFFI_DisplayTxOverviewOutput;

typedef struct VecFFI_DisplayTxOverviewOutput *PtrT_VecFFI_DisplayTxOverviewOutput;

typedef struct DisplayTxOverview {
  PtrString total_output_amount;
  PtrString fee_amount;
  PtrString total_output_sat;
  PtrString fee_sat;
  PtrT_VecFFI_DisplayTxOverviewInput from;
  PtrT_VecFFI_DisplayTxOverviewOutput to;
  PtrString network;
  bool is_multisig;
  bool fee_larger_than_amount;
  PtrString sign_status;
  bool need_sign;
} DisplayTxOverview;

typedef struct DisplayTxDetailInput {
  bool has_address;
  PtrString address;
  PtrString amount;
  bool is_mine;
  PtrString path;
  bool is_external;
} DisplayTxDetailInput;

typedef struct DisplayTxDetailInput *PtrT_DisplayTxDetailInput;

typedef struct VecFFI_DisplayTxDetailInput {
  PtrT_DisplayTxDetailInput data;
  size_t size;
  size_t cap;
} VecFFI_DisplayTxDetailInput;

typedef struct VecFFI_DisplayTxDetailInput *PtrT_VecFFI_DisplayTxDetailInput;

typedef struct DisplayTxDetailOutput {
  PtrString address;
  PtrString amount;
  bool is_mine;
  PtrString path;
  bool is_external;
} DisplayTxDetailOutput;

typedef struct DisplayTxDetailOutput *PtrT_DisplayTxDetailOutput;

typedef struct VecFFI_DisplayTxDetailOutput {
  PtrT_DisplayTxDetailOutput data;
  size_t size;
  size_t cap;
} VecFFI_DisplayTxDetailOutput;

typedef struct VecFFI_DisplayTxDetailOutput *PtrT_VecFFI_DisplayTxDetailOutput;

typedef struct DisplayTxDetail {
  PtrString total_input_amount;
  PtrString total_output_amount;
  PtrString fee_amount;
  PtrT_VecFFI_DisplayTxDetailInput from;
  PtrT_VecFFI_DisplayTxDetailOutput to;
  PtrString network;
  PtrString total_input_sat;
  PtrString total_output_sat;
  PtrString fee_sat;
  PtrString sign_status;
} DisplayTxDetail;

typedef struct DisplayTx {
  struct DisplayTxOverview *overview;
  struct DisplayTxDetail *detail;
} DisplayTx;

typedef struct TransactionParseResult_DisplayTx {
  struct DisplayTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayTx;

typedef struct DisplayBtcMsg {
  PtrString detail;
  PtrString address;
} DisplayBtcMsg;

typedef struct TransactionParseResult_DisplayBtcMsg {
  struct DisplayBtcMsg *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayBtcMsg;

typedef struct TransactionParseResult_DisplayBtcMsg *Ptr_TransactionParseResult_DisplayBtcMsg;

typedef PtrString *PtrT_PtrString;

typedef struct VecFFI_PtrString {
  PtrT_PtrString data;
  size_t size;
  size_t cap;
} VecFFI_PtrString;

typedef struct VecFFI_PtrString *PtrT_VecFFI_PtrString;

typedef struct MultiSigXPubItem {
  PtrString xfp;
  PtrString xpub;
} MultiSigXPubItem;

typedef struct MultiSigXPubItem *PtrT_MultiSigXPubItem;

typedef struct VecFFI_MultiSigXPubItem {
  PtrT_MultiSigXPubItem data;
  size_t size;
  size_t cap;
} VecFFI_MultiSigXPubItem;

typedef struct VecFFI_MultiSigXPubItem *PtrT_VecFFI_MultiSigXPubItem;

typedef struct MultiSigWallet {
  PtrString creator;
  PtrString name;
  PtrString policy;
  uint32_t threshold;
  uint32_t total;
  PtrT_VecFFI_PtrString derivations;
  PtrString format;
  PtrT_VecFFI_MultiSigXPubItem xpub_items;
  PtrString verify_code;
  PtrString config_text;
  uint32_t network;
} MultiSigWallet;

typedef struct MultiSigWallet *PtrT_MultiSigWallet;

typedef struct Response_MultiSigWallet {
  struct MultiSigWallet *data;
  uint32_t error_code;
  PtrString error_message;
} Response_MultiSigWallet;

typedef struct Response_MultiSigWallet *Ptr_Response_MultiSigWallet;

typedef struct SimpleResponse_c_char *Ptr_SimpleResponse_c_char;

typedef struct MultiSigXPubInfoItem {
  PtrString path;
  PtrString xfp;
  PtrString xpub;
} MultiSigXPubInfoItem;

typedef struct MultiSigXPubInfoItem *PtrT_MultiSigXPubInfoItem;

typedef struct MultisigSignResult {
  Ptr_UREncodeResult ur_result;
  PtrString sign_status;
  bool is_completed;
  PtrBytes psbt_hex;
  uint32_t psbt_len;
} MultisigSignResult;

typedef struct MultisigSignResult *PtrT_MultisigSignResult;

typedef struct PsbtSignResult {
  PtrString base_str;
  PtrString hex_str;
  PtrT_UREncodeResult ur_result;
} PsbtSignResult;

typedef struct Response_PsbtSignResult {
  struct PsbtSignResult *data;
  uint32_t error_code;
  PtrString error_message;
} Response_PsbtSignResult;

typedef struct Response_PsbtSignResult *PtrT_Response_PsbtSignResult;

typedef struct TransactionParseResult_DisplayTx *PtrT_TransactionParseResult_DisplayTx;

typedef struct TransactionParseResult_DisplayBtcMsg *PtrT_TransactionParseResult_DisplayBtcMsg;

typedef struct VecFFI_PtrString *Ptr_VecFFI_PtrString;

typedef struct DisplayCardanoSignTxHash {
  PtrString network;
  Ptr_VecFFI_PtrString path;
  PtrString tx_hash;
  Ptr_VecFFI_PtrString address_list;
} DisplayCardanoSignTxHash;

typedef struct TransactionParseResult_DisplayCardanoSignTxHash {
  struct DisplayCardanoSignTxHash *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayCardanoSignTxHash;

typedef struct TransactionParseResult_DisplayCardanoSignTxHash *PtrT_TransactionParseResult_DisplayCardanoSignTxHash;

typedef struct DisplayCardanoSignData {
  PtrString payload;
  PtrString derivation_path;
  PtrString message_hash;
  PtrString xpub;
} DisplayCardanoSignData;

typedef struct TransactionParseResult_DisplayCardanoSignData {
  struct DisplayCardanoSignData *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayCardanoSignData;

typedef struct TransactionParseResult_DisplayCardanoSignData *PtrT_TransactionParseResult_DisplayCardanoSignData;

typedef struct DisplayCardanoCatalyst {
  PtrString nonce;
  PtrString stake_key;
  PtrString rewards;
  Ptr_VecFFI_PtrString vote_keys;
} DisplayCardanoCatalyst;

typedef struct TransactionParseResult_DisplayCardanoCatalyst {
  struct DisplayCardanoCatalyst *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayCardanoCatalyst;

typedef struct TransactionParseResult_DisplayCardanoCatalyst *PtrT_TransactionParseResult_DisplayCardanoCatalyst;

typedef struct DisplayCardanoFrom {
  PtrString address;
  PtrString amount;
  bool has_path;
  PtrString path;
} DisplayCardanoFrom;

typedef struct DisplayCardanoFrom *PtrT_DisplayCardanoFrom;

typedef struct VecFFI_DisplayCardanoFrom {
  PtrT_DisplayCardanoFrom data;
  size_t size;
  size_t cap;
} VecFFI_DisplayCardanoFrom;

typedef struct VecFFI_DisplayCardanoFrom *PtrT_VecFFI_DisplayCardanoFrom;

typedef struct DisplayCardanoTo {
  PtrString address;
  PtrString amount;
  bool has_assets;
  PtrString assets_text;
} DisplayCardanoTo;

typedef struct DisplayCardanoTo *PtrT_DisplayCardanoTo;

typedef struct VecFFI_DisplayCardanoTo {
  PtrT_DisplayCardanoTo data;
  size_t size;
  size_t cap;
} VecFFI_DisplayCardanoTo;

typedef struct VecFFI_DisplayCardanoTo *PtrT_VecFFI_DisplayCardanoTo;

typedef struct DisplayCertField {
  PtrString label;
  PtrString value;
} DisplayCertField;

typedef struct DisplayCertField *PtrT_DisplayCertField;

typedef struct VecFFI_DisplayCertField {
  PtrT_DisplayCertField data;
  size_t size;
  size_t cap;
} VecFFI_DisplayCertField;

typedef struct VecFFI_DisplayCertField *Ptr_VecFFI_DisplayCertField;

typedef struct DisplayCardanoCertificate {
  PtrString cert_type;
  Ptr_VecFFI_DisplayCertField fields;
} DisplayCardanoCertificate;

typedef struct DisplayCardanoCertificate *PtrT_DisplayCardanoCertificate;

typedef struct VecFFI_DisplayCardanoCertificate {
  PtrT_DisplayCardanoCertificate data;
  size_t size;
  size_t cap;
} VecFFI_DisplayCardanoCertificate;

typedef struct VecFFI_DisplayCardanoCertificate *Ptr_VecFFI_DisplayCardanoCertificate;

typedef struct DisplayCardanoWithdrawal {
  PtrString address;
  PtrString amount;
} DisplayCardanoWithdrawal;

typedef struct DisplayCardanoWithdrawal *PtrT_DisplayCardanoWithdrawal;

typedef struct VecFFI_DisplayCardanoWithdrawal {
  PtrT_DisplayCardanoWithdrawal data;
  size_t size;
  size_t cap;
} VecFFI_DisplayCardanoWithdrawal;

typedef struct VecFFI_DisplayCardanoWithdrawal *Ptr_VecFFI_DisplayCardanoWithdrawal;

typedef struct DisplayVotingProcedure {
  PtrString voter;
  PtrString transaction_id;
  PtrString index;
  PtrString vote;
} DisplayVotingProcedure;

typedef struct DisplayVotingProcedure *PtrT_DisplayVotingProcedure;

typedef struct VecFFI_DisplayVotingProcedure {
  PtrT_DisplayVotingProcedure data;
  size_t size;
  size_t cap;
} VecFFI_DisplayVotingProcedure;

typedef struct VecFFI_DisplayVotingProcedure *Ptr_VecFFI_DisplayVotingProcedure;

typedef struct DisplayVotingProposal {
  PtrString anchor;
} DisplayVotingProposal;

typedef struct DisplayVotingProposal *PtrT_DisplayVotingProposal;

typedef struct VecFFI_DisplayVotingProposal {
  PtrT_DisplayVotingProposal data;
  size_t size;
  size_t cap;
} VecFFI_DisplayVotingProposal;

typedef struct VecFFI_DisplayVotingProposal *Ptr_VecFFI_DisplayVotingProposal;

typedef struct DisplayCardanoTx {
  PtrT_VecFFI_DisplayCardanoFrom from;
  PtrT_VecFFI_DisplayCardanoTo to;
  PtrString fee;
  PtrString network;
  PtrString total_input;
  PtrString total_output;
  Ptr_VecFFI_DisplayCardanoCertificate certificates;
  Ptr_VecFFI_DisplayCardanoWithdrawal withdrawals;
  PtrString auxiliary_data;
  Ptr_VecFFI_DisplayVotingProcedure voting_procedures;
  Ptr_VecFFI_DisplayVotingProposal voting_proposals;
} DisplayCardanoTx;

typedef struct TransactionParseResult_DisplayCardanoTx {
  struct DisplayCardanoTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayCardanoTx;

typedef struct TransactionParseResult_DisplayCardanoTx *PtrT_TransactionParseResult_DisplayCardanoTx;

typedef struct DisplayCosmosTxOverview {
  PtrString display_type;
  PtrString method;
  PtrString network;
  PtrString send_value;
  PtrString send_from;
  PtrString send_to;
  PtrString delegate_value;
  PtrString delegate_from;
  PtrString delegate_to;
  PtrString undelegate_value;
  PtrString undelegate_to;
  PtrString undelegate_validator;
  PtrString redelegate_value;
  PtrString redelegate_to;
  PtrString redelegate_new_validator;
  PtrString withdraw_reward_to;
  PtrString withdraw_reward_validator;
  PtrString transfer_from;
  PtrString transfer_to;
  PtrString transfer_value;
  PtrString vote_voted;
  PtrString vote_proposal;
  PtrString vote_voter;
  PtrString overview_list;
} DisplayCosmosTxOverview;

typedef struct DisplayCosmosTxOverview *PtrT_DisplayCosmosTxOverview;

typedef struct DisplayCosmosTx {
  PtrT_DisplayCosmosTxOverview overview;
  PtrString detail;
} DisplayCosmosTx;

typedef struct TransactionParseResult_DisplayCosmosTx {
  struct DisplayCosmosTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayCosmosTx;

typedef struct TransactionParseResult_DisplayCosmosTx *PtrT_TransactionParseResult_DisplayCosmosTx;

typedef struct DisplayETHOverview {
  PtrString value;
  PtrString max_txn_fee;
  PtrString gas_price;
  PtrString gas_limit;
  PtrString from;
  PtrString to;
} DisplayETHOverview;

typedef struct DisplayETHOverview *PtrT_DisplayETHOverview;

typedef struct DisplayETHDetail {
  PtrString value;
  PtrString max_txn_fee;
  PtrString max_fee;
  PtrString max_priority;
  PtrString max_fee_price;
  PtrString max_priority_price;
  PtrString gas_price;
  PtrString gas_limit;
  PtrString from;
  PtrString to;
  PtrString input;
} DisplayETHDetail;

typedef struct DisplayETHDetail *PtrT_DisplayETHDetail;

typedef struct DisplayETH {
  PtrString tx_type;
  uint64_t chain_id;
  PtrT_DisplayETHOverview overview;
  PtrT_DisplayETHDetail detail;
} DisplayETH;

typedef struct TransactionParseResult_DisplayETH {
  struct DisplayETH *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayETH;

typedef struct TransactionParseResult_DisplayETH *PtrT_TransactionParseResult_DisplayETH;

typedef struct DisplayETHPersonalMessage {
  PtrString raw_message;
  PtrString utf8_message;
  PtrString from;
} DisplayETHPersonalMessage;

typedef struct TransactionParseResult_DisplayETHPersonalMessage {
  struct DisplayETHPersonalMessage *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayETHPersonalMessage;

typedef struct TransactionParseResult_DisplayETHPersonalMessage *PtrT_TransactionParseResult_DisplayETHPersonalMessage;

typedef struct DisplayETHTypedData {
  PtrString name;
  PtrString version;
  PtrString chain_id;
  PtrString verifying_contract;
  PtrString salt;
  PtrString primary_type;
  PtrString message;
  PtrString from;
} DisplayETHTypedData;

typedef struct TransactionParseResult_DisplayETHTypedData {
  struct DisplayETHTypedData *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayETHTypedData;

typedef struct TransactionParseResult_DisplayETHTypedData *PtrT_TransactionParseResult_DisplayETHTypedData;

typedef struct EthParsedErc20Transaction {
  PtrString to;
  PtrString value;
} EthParsedErc20Transaction;

typedef struct TransactionParseResult_EthParsedErc20Transaction {
  struct EthParsedErc20Transaction *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_EthParsedErc20Transaction;

typedef struct TransactionParseResult_EthParsedErc20Transaction *PtrT_TransactionParseResult_EthParsedErc20Transaction;

typedef struct DisplayContractParam {
  PtrString name;
  PtrString value;
} DisplayContractParam;

typedef struct DisplayContractParam *PtrT_DisplayContractParam;

typedef struct VecFFI_DisplayContractParam {
  PtrT_DisplayContractParam data;
  size_t size;
  size_t cap;
} VecFFI_DisplayContractParam;

typedef struct VecFFI_DisplayContractParam *PtrT_VecFFI_DisplayContractParam;

typedef struct DisplayContractData {
  PtrString contract_name;
  PtrString method_name;
  PtrT_VecFFI_DisplayContractParam params;
} DisplayContractData;

typedef struct Response_DisplayContractData {
  struct DisplayContractData *data;
  uint32_t error_code;
  PtrString error_message;
} Response_DisplayContractData;

typedef struct Response_DisplayContractData *Ptr_Response_DisplayContractData;

typedef struct Response_DisplayContractData *PtrT_Response_DisplayContractData;

typedef struct DisplayNearTxOverviewGeneralAction {
  PtrString action;
} DisplayNearTxOverviewGeneralAction;

typedef struct DisplayNearTxOverviewGeneralAction *PtrT_DisplayNearTxOverviewGeneralAction;

typedef struct VecFFI_DisplayNearTxOverviewGeneralAction {
  PtrT_DisplayNearTxOverviewGeneralAction data;
  size_t size;
  size_t cap;
} VecFFI_DisplayNearTxOverviewGeneralAction;

typedef struct VecFFI_DisplayNearTxOverviewGeneralAction *PtrT_VecFFI_DisplayNearTxOverviewGeneralAction;

typedef struct DisplayNearTxOverview {
  PtrString display_type;
  PtrString main_action;
  PtrString transfer_value;
  PtrString transfer_from;
  PtrString transfer_to;
  PtrT_VecFFI_DisplayNearTxOverviewGeneralAction action_list;
} DisplayNearTxOverview;

typedef struct DisplayNearTxOverview *PtrT_DisplayNearTxOverview;

typedef struct DisplayNearTx {
  PtrString network;
  PtrT_DisplayNearTxOverview overview;
  PtrString detail;
} DisplayNearTx;

typedef struct TransactionParseResult_DisplayNearTx {
  struct DisplayNearTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayNearTx;

typedef struct TransactionParseResult_DisplayNearTx *PtrT_TransactionParseResult_DisplayNearTx;

typedef struct DisplaySolanaTxOverviewVotesOn {
  PtrString slot;
} DisplaySolanaTxOverviewVotesOn;

typedef struct DisplaySolanaTxOverviewVotesOn *PtrT_DisplaySolanaTxOverviewVotesOn;

typedef struct VecFFI_DisplaySolanaTxOverviewVotesOn {
  PtrT_DisplaySolanaTxOverviewVotesOn data;
  size_t size;
  size_t cap;
} VecFFI_DisplaySolanaTxOverviewVotesOn;

typedef struct VecFFI_DisplaySolanaTxOverviewVotesOn *PtrT_VecFFI_DisplaySolanaTxOverviewVotesOn;

typedef struct DisplaySolanaTxOverviewGeneral {
  PtrString program;
  PtrString method;
} DisplaySolanaTxOverviewGeneral;

typedef struct DisplaySolanaTxOverviewGeneral *PtrT_DisplaySolanaTxOverviewGeneral;

typedef struct VecFFI_DisplaySolanaTxOverviewGeneral {
  PtrT_DisplaySolanaTxOverviewGeneral data;
  size_t size;
  size_t cap;
} VecFFI_DisplaySolanaTxOverviewGeneral;

typedef struct VecFFI_DisplaySolanaTxOverviewGeneral *PtrT_VecFFI_DisplaySolanaTxOverviewGeneral;

typedef struct Instruction {
  PtrT_VecFFI_PtrString accounts;
  PtrString data;
  PtrString program_address;
} Instruction;

typedef struct Instruction *PtrT_Instruction;

typedef struct VecFFI_Instruction {
  PtrT_Instruction data;
  size_t size;
  size_t cap;
} VecFFI_Instruction;

typedef struct VecFFI_Instruction *PtrT_VecFFI_Instruction;

typedef struct DisplaySolanaTxOverviewUnknownInstructions {
  PtrT_VecFFI_PtrString overview_accounts;
  PtrT_VecFFI_Instruction overview_instructions;
} DisplaySolanaTxOverviewUnknownInstructions;

typedef struct DisplaySolanaTxOverviewUnknownInstructions *PtrT_DisplaySolanaTxOverviewUnknownInstructions;

typedef struct ProgramOverviewTransfer {
  PtrString value;
  PtrString main_action;
  PtrString from;
  PtrString to;
} ProgramOverviewTransfer;

typedef struct ProgramOverviewTransfer *PtrT_ProgramOverviewTransfer;

typedef struct VecFFI_ProgramOverviewTransfer {
  PtrT_ProgramOverviewTransfer data;
  size_t size;
  size_t cap;
} VecFFI_ProgramOverviewTransfer;

typedef struct VecFFI_ProgramOverviewTransfer *PtrT_VecFFI_ProgramOverviewTransfer;

typedef struct DisplaySolanaTxOverviewSquadsV4MultisigCreate {
  PtrString wallet_name;
  PtrString wallet_desc;
  uint16_t threshold;
  uintptr_t member_count;
  PtrT_VecFFI_PtrString members;
  PtrString total_value;
  PtrT_VecFFI_ProgramOverviewTransfer transfers;
} DisplaySolanaTxOverviewSquadsV4MultisigCreate;

typedef struct DisplaySolanaTxOverviewSquadsV4MultisigCreate *PtrT_DisplaySolanaTxOverviewSquadsV4MultisigCreate;

typedef struct DisplaySolanaTxProposalOverview {
  PtrString program;
  PtrString method;
  PtrString memo;
  PtrString data;
} DisplaySolanaTxProposalOverview;

typedef struct DisplaySolanaTxProposalOverview *PtrT_DisplaySolanaTxProposalOverview;

typedef struct VecFFI_DisplaySolanaTxProposalOverview {
  PtrT_DisplaySolanaTxProposalOverview data;
  size_t size;
  size_t cap;
} VecFFI_DisplaySolanaTxProposalOverview;

typedef struct VecFFI_DisplaySolanaTxProposalOverview *PtrT_VecFFI_DisplaySolanaTxProposalOverview;

typedef struct DisplaySolanaTxSplTokenTransferOverview {
  PtrString source;
  PtrString destination;
  PtrString authority;
  uint8_t decimals;
  PtrString amount;
  PtrString token_mint_account;
  PtrString token_symbol;
  PtrString token_name;
} DisplaySolanaTxSplTokenTransferOverview;

typedef struct DisplaySolanaTxSplTokenTransferOverview *PtrT_DisplaySolanaTxSplTokenTransferOverview;

typedef struct JupiterV6SwapTokenInfoOverview {
  PtrString token_name;
  PtrString token_symbol;
  PtrString token_address;
  PtrString token_amount;
  bool exist_in_address_lookup_table;
} JupiterV6SwapTokenInfoOverview;

typedef struct JupiterV6SwapTokenInfoOverview *PtrT_JupiterV6SwapTokenInfoOverview;

typedef struct DisplaySolanaTxOverviewJupiterV6Swap {
  PtrString program_name;
  PtrString program_address;
  PtrString instruction_name;
  PtrT_JupiterV6SwapTokenInfoOverview token_a_overview;
  PtrT_JupiterV6SwapTokenInfoOverview token_b_overview;
  PtrString slippage_bps;
  PtrString platform_fee_bps;
} DisplaySolanaTxOverviewJupiterV6Swap;

typedef struct DisplaySolanaTxOverviewJupiterV6Swap *PtrT_DisplaySolanaTxOverviewJupiterV6Swap;

typedef struct DisplaySolanaTxOverview {
  PtrString display_type;
  PtrString main_action;
  PtrString transfer_value;
  PtrString transfer_from;
  PtrString transfer_to;
  PtrT_VecFFI_DisplaySolanaTxOverviewVotesOn votes_on;
  PtrString vote_account;
  PtrT_VecFFI_DisplaySolanaTxOverviewGeneral general;
  PtrT_DisplaySolanaTxOverviewUnknownInstructions unknown_instructions;
  PtrT_DisplaySolanaTxOverviewSquadsV4MultisigCreate squads_multisig_create;
  PtrT_VecFFI_DisplaySolanaTxProposalOverview squads_proposal;
  PtrT_DisplaySolanaTxSplTokenTransferOverview spl_token_transfer;
  PtrT_DisplaySolanaTxOverviewJupiterV6Swap jupiter_v6_swap;
} DisplaySolanaTxOverview;

typedef struct DisplaySolanaTxOverview *PtrT_DisplaySolanaTxOverview;

typedef struct DisplaySolanaTx {
  PtrString network;
  PtrT_DisplaySolanaTxOverview overview;
  PtrString detail;
} DisplaySolanaTx;

typedef struct TransactionParseResult_DisplaySolanaTx {
  struct DisplaySolanaTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplaySolanaTx;

typedef struct TransactionParseResult_DisplaySolanaTx *PtrT_TransactionParseResult_DisplaySolanaTx;

typedef struct DisplaySolanaMessage {
  PtrString raw_message;
  PtrString utf8_message;
  PtrString from;
} DisplaySolanaMessage;

typedef struct TransactionParseResult_DisplaySolanaMessage {
  struct DisplaySolanaMessage *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplaySolanaMessage;

typedef struct TransactionParseResult_DisplaySolanaMessage *PtrT_TransactionParseResult_DisplaySolanaMessage;

typedef struct DisplayStellarTx {
  PtrString raw_message;
} DisplayStellarTx;

typedef struct TransactionParseResult_DisplayStellarTx {
  struct DisplayStellarTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayStellarTx;

typedef struct TransactionParseResult_DisplayStellarTx *PtrT_TransactionParseResult_DisplayStellarTx;

typedef struct DisplaySuiIntentMessage {
  PtrString detail;
} DisplaySuiIntentMessage;

typedef struct TransactionParseResult_DisplaySuiIntentMessage {
  struct DisplaySuiIntentMessage *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplaySuiIntentMessage;

typedef struct TransactionParseResult_DisplaySuiIntentMessage *PtrT_TransactionParseResult_DisplaySuiIntentMessage;

typedef struct DisplaySuiSignMessageHash {
  PtrString network;
  PtrString path;
  PtrString from_address;
  PtrString message;
} DisplaySuiSignMessageHash;

typedef struct TransactionParseResult_DisplaySuiSignMessageHash {
  struct DisplaySuiSignMessageHash *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplaySuiSignMessageHash;

typedef struct TransactionParseResult_DisplaySuiSignMessageHash *PtrT_TransactionParseResult_DisplaySuiSignMessageHash;

typedef struct DisplayTonTransaction {
  PtrString amount;
  PtrString action;
  PtrString to;
  PtrString comment;
  PtrString data_view;
  PtrString raw_data;
  PtrString contract_data;
} DisplayTonTransaction;

typedef struct TransactionParseResult_DisplayTonTransaction {
  struct DisplayTonTransaction *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayTonTransaction;

typedef struct TransactionParseResult_DisplayTonTransaction *PtrT_TransactionParseResult_DisplayTonTransaction;

typedef struct DisplayTonProof {
  PtrString domain;
  PtrString payload;
  PtrString address;
  PtrString raw_message;
} DisplayTonProof;

typedef struct TransactionParseResult_DisplayTonProof {
  struct DisplayTonProof *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayTonProof;

typedef struct TransactionParseResult_DisplayTonProof *PtrT_TransactionParseResult_DisplayTonProof;

typedef struct VecFFI_u8 *Ptr_VecFFI_u8;

typedef struct DisplayTronOverview {
  PtrString value;
  PtrString method;
  PtrString from;
  PtrString to;
  PtrString network;
} DisplayTronOverview;

typedef struct DisplayTronDetail {
  PtrString value;
  PtrString method;
  PtrString from;
  PtrString to;
  PtrString network;
  PtrString token;
  PtrString contract_address;
} DisplayTronDetail;

typedef struct DisplayTron {
  struct DisplayTronOverview *overview;
  struct DisplayTronDetail *detail;
} DisplayTron;

typedef struct TransactionParseResult_DisplayTron {
  struct DisplayTron *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayTron;

typedef struct TransactionParseResult_DisplayTron *PtrT_TransactionParseResult_DisplayTron;

typedef struct DisplayXrpTxOverview {
  PtrString display_type;
  PtrString transaction_type;
  PtrString from;
  PtrString fee;
  PtrString sequence;
  PtrString value;
  PtrString to;
} DisplayXrpTxOverview;

typedef struct DisplayXrpTxOverview *PtrT_DisplayXrpTxOverview;

typedef struct DisplayXrpTx {
  PtrString network;
  PtrT_DisplayXrpTxOverview overview;
  PtrString detail;
  PtrString signing_pubkey;
} DisplayXrpTx;

typedef struct TransactionParseResult_DisplayXrpTx {
  struct DisplayXrpTx *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayXrpTx;

typedef struct TransactionParseResult_DisplayXrpTx *PtrT_TransactionParseResult_DisplayXrpTx;

typedef struct DisplayFrom {
  PtrString address;
  PtrString value;
  bool is_mine;
} DisplayFrom;

typedef struct DisplayFrom *PtrT_DisplayFrom;

typedef struct VecFFI_DisplayFrom {
  PtrT_DisplayFrom data;
  size_t size;
  size_t cap;
} VecFFI_DisplayFrom;

typedef struct VecFFI_DisplayFrom *Ptr_VecFFI_DisplayFrom;

typedef struct DisplayTo {
  PtrString address;
  PtrString value;
  bool is_change;
  PtrString memo;
} DisplayTo;

typedef struct DisplayTo *PtrT_DisplayTo;

typedef struct VecFFI_DisplayTo {
  PtrT_DisplayTo data;
  size_t size;
  size_t cap;
} VecFFI_DisplayTo;

typedef struct VecFFI_DisplayTo *Ptr_VecFFI_DisplayTo;

typedef struct DisplayTransparent {
  Ptr_VecFFI_DisplayFrom from;
  Ptr_VecFFI_DisplayTo to;
} DisplayTransparent;

typedef struct DisplayTransparent *Ptr_DisplayTransparent;

typedef struct DisplayOrchard {
  Ptr_VecFFI_DisplayFrom from;
  Ptr_VecFFI_DisplayTo to;
} DisplayOrchard;

typedef struct DisplayOrchard *Ptr_DisplayOrchard;

typedef struct DisplayPczt {
  Ptr_DisplayTransparent transparent;
  Ptr_DisplayOrchard orchard;
  PtrString total_transfer_value;
  PtrString fee_value;
  bool has_sapling;
} DisplayPczt;

typedef struct TransactionParseResult_DisplayPczt {
  struct DisplayPczt *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplayPczt;

typedef struct TransactionParseResult_DisplayPczt *Ptr_TransactionParseResult_DisplayPczt;

typedef struct TransactionParseResult_DisplayPczt *PtrT_TransactionParseResult_DisplayPczt;

extern const uintptr_t FRAGMENT_MAX_LENGTH_DEFAULT;

extern void LogRustMalloc(void *p, uint32_t size);

extern void LogRustFree(void *p);

extern void LogRustPanic(char *p);

extern int32_t GenerateTRNGRandomness(uint8_t *randomness, uint8_t len);

extern void *RustMalloc(int32_t size);

extern void RustFree(void *p);

struct SimpleResponse_u8 *get_master_fingerprint(PtrBytes seed, uint32_t seed_len);

enum ErrorCodes dummy_function_to_export_error_codes(void);

struct SimpleResponse_c_char *get_extended_pubkey_by_seed(PtrBytes seed,
                                                          uint32_t seed_len,
                                                          PtrString path);

struct SimpleResponse_c_char *get_extended_pubkey_bytes_by_seed(PtrBytes seed,
                                                                uint32_t seed_len,
                                                                PtrString path);

struct SimpleResponse_c_char *get_ed25519_pubkey_by_seed(PtrBytes seed,
                                                         uint32_t seed_len,
                                                         PtrString path);

struct SimpleResponse_u8 *get_rsa_pubkey_by_seed(PtrBytes seed, uint32_t seed_len);

struct SimpleResponse_c_char *get_bip32_ed25519_extended_pubkey(PtrBytes entropy,
                                                                uint32_t entropy_len,
                                                                PtrString passphrase,
                                                                PtrString path);

struct SimpleResponse_c_char *get_ledger_bitbox02_master_key(PtrString mnemonic,
                                                             PtrString passphrase);

struct SimpleResponse_c_char *get_icarus_master_key(PtrBytes entropy,
                                                    uint32_t entropy_len,
                                                    PtrString passphrase);

struct SimpleResponse_c_char *derive_bip32_ed25519_extended_pubkey(PtrString master_key,
                                                                   PtrString path);

struct SimpleResponse_c_char *k1_sign_message_hash_by_private_key(PtrBytes private_key,
                                                                  PtrBytes message_hash);

bool k1_verify_signature(PtrBytes signature, PtrBytes message_hash, PtrBytes public_key);

struct SimpleResponse_u8 *k1_generate_ecdh_sharekey(PtrBytes privkey,
                                                    uint32_t privkey_len,
                                                    PtrBytes pubkey,
                                                    uint32_t pubkey_len);

struct SimpleResponse_u8 *k1_generate_pubkey_by_privkey(PtrBytes privkey, uint32_t privkey_len);

struct SimpleResponse_u8 *pbkdf2_rust(PtrBytes password, PtrBytes salt, uint32_t iterations);

struct SimpleResponse_u8 *pbkdf2_rust_64(PtrBytes password, PtrBytes salt, uint32_t iterations);

Ptr_TransactionCheckResult tx_check_pass(void);

struct SimpleResponse_c_char *rust_aes256_cbc_encrypt(PtrString data,
                                                      PtrString password,
                                                      PtrBytes iv,
                                                      uint32_t iv_len);

struct SimpleResponse_c_char *rust_aes256_cbc_decrypt(PtrString hex_data,
                                                      PtrString password,
                                                      PtrBytes iv,
                                                      uint32_t iv_len);

struct SimpleResponse_u8 *rust_derive_iv_from_seed(PtrBytes seed, uint32_t seed_len);

void free_ur_parse_result(PtrT_URParseResult ur_parse_result);

void free_ur_parse_multi_result(PtrT_URParseMultiResult ptr);

void free_ur_encode_result(PtrT_UREncodeResult ptr);

void free_ur_encode_muilt_result(PtrT_UREncodeMultiResult ptr);

void free_simple_response_u8(PtrT_SimpleResponse_u8 ptr);

void free_simple_response_c_char(PtrT_SimpleResponse_c_char ptr);

void free_ptr_string(PtrString ptr);

void free_rust_value(void *any_ptr);

void free_VecFFI_u8(PtrT_VecFFI_u8 ptr);

enum QRProtocol infer_qrcode_type(PtrString qrcode);

Ptr_URParseResult parse_qrcode_text(PtrString qr);

void free_TransactionCheckResult(PtrT_TransactionCheckResult ptr);

struct UREncodeMultiResult *get_next_part(PtrEncoder ptr);

struct URParseResult *parse_ur(PtrString ur);

struct URParseMultiResult *receive(PtrString ur, PtrDecoder decoder);

PtrString calculate_auth_code(ConstPtrUR web_auth_data,
                              PtrBytes rsa_key_n,
                              uint32_t rsa_key_n_len,
                              PtrBytes rsa_key_d,
                              uint32_t rsa_key_d_len);

struct UREncodeResult *get_connect_blue_wallet_ur(uint8_t *master_fingerprint,
                                                  uint32_t length,
                                                  PtrT_CSliceFFI_ExtendedPublicKey public_keys);

struct UREncodeResult *get_connect_sparrow_wallet_ur(uint8_t *master_fingerprint,
                                                     uint32_t length,
                                                     PtrT_CSliceFFI_ExtendedPublicKey public_keys);

struct UREncodeResult *get_connect_specter_wallet_ur(uint8_t *master_fingerprint,
                                                     uint32_t length,
                                                     PtrT_CSliceFFI_ExtendedPublicKey public_keys);

PtrT_UREncodeResult get_connect_companion_app_ur(PtrBytes master_fingerprint,
                                                 uint32_t master_fingerprint_length,
                                                 int32_t cold_version,
                                                 PtrT_CoinConfig coin_config,
                                                 uint32_t coin_config_length);

Ptr_UREncodeResult get_okx_wallet_ur_btc_only(PtrBytes master_fingerprint,
                                              uint32_t master_fingerprint_length,
                                              PtrString serial_number,
                                              Ptr_CSliceFFI_ExtendedPublicKey public_keys,
                                              PtrString device_type,
                                              PtrString device_version);

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_metamask_ur_dynamic(PtrBytes master_fingerprint,
                                                       uint32_t master_fingerprint_length,
                                                       enum ETHAccountType account_type,
                                                       PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                                       uintptr_t fragment_max_length_default,
                                                       uintptr_t fragment_max_length_other);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_metamask_ur_unlimited(PtrBytes master_fingerprint,
                                                         uint32_t master_fingerprint_length,
                                                         enum ETHAccountType account_type,
                                                         PtrT_CSliceFFI_ExtendedPublicKey public_keys);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_metamask_ur(PtrBytes master_fingerprint,
                                               uint32_t master_fingerprint_length,
                                               enum ETHAccountType account_type,
                                               PtrT_CSliceFFI_ExtendedPublicKey public_keys);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_Response_QRHardwareCallData parse_qr_hardware_call(PtrUR ur);
#endif

#if defined(BUILD_MULTI_COINS)
struct Response_bool *check_hardware_call_path(PtrString path, PtrString chain_type);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult generate_key_derivation_ur(PtrBytes master_fingerprint,
                                              uint32_t master_fingerprint_length,
                                              Ptr_CSliceFFI_ExtendedPublicKey xpubs,
                                              PtrString device_version);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_aptos_wallet_ur(uint8_t *master_fingerprint,
                                                   uint32_t length,
                                                   PtrT_CSliceFFI_ExtendedPublicKey public_keys);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_connect_arconnect_wallet_ur(PtrBytes master_fingerprint,
                                                   uint32_t master_fingerprint_length,
                                                   PtrBytes p,
                                                   uint32_t p_len,
                                                   PtrBytes q,
                                                   uint32_t q_len);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_connect_arconnect_wallet_ur_from_xpub(PtrBytes master_fingerprint,
                                                             uint32_t master_fingerprint_length,
                                                             PtrString xpub);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_backpack_wallet_ur(PtrBytes master_fingerprint,
                                          uint32_t master_fingerprint_length,
                                          Ptr_CSliceFFI_ExtendedPublicKey public_keys);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_bitget_wallet_ur(PtrBytes master_fingerprint,
                                        uint32_t master_fingerprint_length,
                                        PtrString serial_number,
                                        Ptr_CSliceFFI_ExtendedPublicKey public_keys,
                                        PtrString device_type,
                                        PtrString device_version);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_connect_imtoken_ur(PtrBytes master_fingerprint,
                                          uint32_t master_fingerprint_length,
                                          PtrString xpub,
                                          PtrString wallet_name);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_keplr_wallet_ur(PtrBytes master_fingerprint,
                                                   uint32_t master_fingerprint_length,
                                                   PtrT_CSliceFFI_KeplrAccount keplr_accounts);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_keystone_wallet_ur(PtrBytes master_fingerprint,
                                          uint32_t master_fingerprint_length,
                                          PtrString serial_number,
                                          Ptr_CSliceFFI_ExtendedPublicKey public_keys,
                                          PtrString device_type,
                                          PtrString device_version);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_okx_wallet_ur(PtrBytes master_fingerprint,
                                     uint32_t master_fingerprint_length,
                                     PtrString serial_number,
                                     Ptr_CSliceFFI_ExtendedPublicKey public_keys,
                                     PtrString device_type,
                                     PtrString device_version);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_solana_wallet_ur(uint8_t *master_fingerprint,
                                                    uint32_t length,
                                                    PtrT_CSliceFFI_ExtendedPublicKey public_keys);
#endif

#if defined(BUILD_MULTI_COINS)
void free_Response_QRHardwareCallData(PtrT_Response_QRHardwareCallData ptr);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_sui_wallet_ur(uint8_t *master_fingerprint,
                                                 uint32_t length,
                                                 PtrT_CSliceFFI_ExtendedPublicKey public_keys);
#endif

#if defined(BUILD_MULTI_COINS)
Ptr_UREncodeResult get_tonkeeper_wallet_ur(PtrString public_key,
                                           PtrString wallet_name,
                                           PtrBytes master_fingerprint,
                                           uint32_t master_fingerprint_length,
                                           PtrString path);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_xbull_wallet_ur(uint8_t *master_fingerprint,
                                                   uint32_t length,
                                                   PtrT_CSliceFFI_ExtendedPublicKey public_keys);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_xrp_toolkit_ur(PtrString hd_path,
                                                  PtrString root_x_pub,
                                                  PtrString root_path);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_zcash_wallet_ur(PtrBytes seed_fingerprint,
                                                   uint32_t seed_fingerprint_len,
                                                   Ptr_CSliceFFI_ZcashKey zcash_keys);
#endif

#if defined(BUILD_MULTI_COINS)
struct UREncodeResult *get_connect_thor_wallet_ur(uint8_t *master_fingerprint,
                                                  uint32_t length,
                                                  PtrString serial_number,
                                                  PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                                  PtrString device_type,
                                                  PtrString device_version);
#endif

struct SimpleResponse_c_char *aptos_generate_address(PtrString pub_key);

PtrT_TransactionCheckResult aptos_check_request(PtrUR ptr,
                                                PtrBytes master_fingerprint,
                                                uint32_t length);

PtrT_TransactionParseResult_DisplayAptosTx aptos_parse(PtrUR ptr);

PtrT_UREncodeResult aptos_sign_tx(PtrUR ptr, PtrBytes seed, uint32_t seed_len, PtrString pub_key);

PtrString aptos_get_path(PtrUR ptr);

struct SimpleResponse_c_char *test_aptos_parse(void);

void free_TransactionParseResult_DisplayAptosTx(PtrT_TransactionParseResult_DisplayAptosTx ptr);

struct SimpleResponse_u8 *generate_arweave_secret(PtrBytes seed, uint32_t seed_len);

struct SimpleResponse_u8 *generate_arweave_public_key_from_primes(PtrBytes p,
                                                                  uint32_t p_len,
                                                                  PtrBytes q,
                                                                  uint32_t q_len);

struct SimpleResponse_c_char *generate_rsa_public_key(PtrBytes p,
                                                      uint32_t p_len,
                                                      PtrBytes q,
                                                      uint32_t q_len);

struct SimpleResponse_u8 *aes256_encrypt_primes(PtrBytes seed, uint32_t seed_len, PtrBytes data);

struct SimpleResponse_u8 *aes256_decrypt_primes(PtrBytes seed, uint32_t seed_len, PtrBytes data);

struct SimpleResponse_c_char *arweave_get_address(PtrString xpub);

struct SimpleResponse_c_char *fix_arweave_address(PtrString address);

PtrT_TransactionCheckResult ar_check_tx(PtrUR ptr, PtrBytes master_fingerprint, uint32_t length);

struct SimpleResponse_ArweaveRequestType *ar_request_type(PtrUR ptr);

PtrT_TransactionParseResult_DisplayArweaveMessage ar_message_parse(PtrUR ptr);

PtrT_TransactionParseResult_DisplayArweaveTx ar_parse(PtrUR ptr);

PtrT_UREncodeResult ar_sign_tx(PtrUR ptr, PtrBytes p, uint32_t p_len, PtrBytes q, uint32_t q_len);

bool ar_is_ao_transfer(PtrUR ptr);

PtrT_TransactionParseResult_DisplayArweaveDataItem ar_parse_data_item(PtrUR ptr);

PtrT_TransactionParseResult_DisplayArweaveAOTransfer ar_parse_ao_transfer(PtrUR ptr);

void free_TransactionParseResult_DisplayArweaveTx(PtrT_TransactionParseResult_DisplayArweaveTx ptr);

void free_TransactionParseResult_DisplayArweaveMessage(PtrT_TransactionParseResult_DisplayArweaveMessage ptr);

struct SimpleResponse_c_char *utxo_get_address(PtrString hd_path, PtrString x_pub);

struct SimpleResponse_c_char *xpub_convert_version(PtrString x_pub, PtrString target);

struct TransactionParseResult_DisplayTx *utxo_parse_keystone(PtrUR ptr,
                                                             enum QRCodeType ur_type,
                                                             PtrBytes master_fingerprint,
                                                             uint32_t length,
                                                             PtrString x_pub);

struct UREncodeResult *utxo_sign_keystone(PtrUR ptr,
                                          enum QRCodeType ur_type,
                                          PtrBytes master_fingerprint,
                                          uint32_t length,
                                          PtrString x_pub,
                                          int32_t cold_version,
                                          PtrBytes seed,
                                          uint32_t seed_len);

PtrT_TransactionCheckResult utxo_check_keystone(PtrUR ptr,
                                                enum QRCodeType ur_type,
                                                PtrBytes master_fingerprint,
                                                uint32_t length,
                                                PtrString x_pub);

PtrT_TransactionCheckResult btc_check_msg(PtrUR ptr, PtrBytes master_fingerprint, uint32_t length);

struct TransactionParseResult_DisplayBtcMsg *btc_parse_msg(PtrUR ptr,
                                                           Ptr_CSliceFFI_ExtendedPublicKey xpubs,
                                                           PtrBytes master_fingerprint,
                                                           uint32_t length);

struct UREncodeResult *btc_sign_msg(PtrUR ptr,
                                    PtrBytes seed,
                                    uint32_t seed_len,
                                    PtrBytes master_fingerprint,
                                    uint32_t master_fingerprint_len);

Ptr_TransactionParseResult_DisplayBtcMsg parse_seed_signer_message(PtrUR ptr,
                                                                   Ptr_CSliceFFI_ExtendedPublicKey xpubs);

struct UREncodeResult *sign_seed_signer_message(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

struct UREncodeResult *export_multi_sig_xpub_by_ur(uint8_t *master_fingerprint,
                                                   uint32_t length,
                                                   PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                                   enum NetworkType network);

struct UREncodeResult *export_multi_sig_wallet_by_ur_test(uint8_t *master_fingerprint,
                                                          uint32_t length,
                                                          PtrT_MultiSigWallet multi_sig_wallet);

struct UREncodeResult *export_multi_sig_wallet_by_ur(uint8_t *master_fingerprint,
                                                     uint32_t length,
                                                     PtrString config);

Ptr_Response_MultiSigWallet import_multi_sig_wallet_by_ur(PtrUR ur,
                                                          PtrBytes master_fingerprint,
                                                          uint32_t master_fingerprint_len);

Ptr_Response_MultiSigWallet import_multi_sig_wallet_by_file(PtrString content,
                                                            PtrBytes master_fingerprint,
                                                            uint32_t master_fingerprint_len);

Ptr_SimpleResponse_c_char generate_address_for_multisig_wallet_config(PtrString wallet_config,
                                                                      uint32_t account,
                                                                      uint32_t index,
                                                                      PtrBytes master_fingerprint,
                                                                      uint32_t master_fingerprint_len);

Ptr_SimpleResponse_c_char generate_psbt_file_name(PtrBytes psbt_hex,
                                                  uint32_t psbt_len,
                                                  uint32_t time_stamp);

Ptr_Response_MultiSigWallet parse_and_verify_multisig_config(PtrBytes seed,
                                                             uint32_t seed_len,
                                                             PtrString wallet_config,
                                                             PtrBytes master_fingerprint,
                                                             uint32_t master_fingerprint_len);

void free_MultiSigXPubInfoItem(PtrT_MultiSigXPubInfoItem ptr);

void free_MultiSigWallet(PtrT_MultiSigWallet ptr);

void free_MultisigSignResult(PtrT_MultisigSignResult ptr);

struct TransactionParseResult_DisplayTx *btc_parse_psbt(PtrUR ptr,
                                                        PtrBytes master_fingerprint,
                                                        uint32_t length,
                                                        PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                                        PtrString multisig_wallet_config);

struct UREncodeResult *btc_sign_psbt(PtrUR ptr,
                                     PtrBytes seed,
                                     uint32_t seed_len,
                                     PtrBytes master_fingerprint,
                                     uint32_t master_fingerprint_len);

struct UREncodeResult *btc_sign_psbt_unlimited(PtrUR ptr,
                                               PtrBytes seed,
                                               uint32_t seed_len,
                                               PtrBytes master_fingerprint,
                                               uint32_t master_fingerprint_len);

struct MultisigSignResult *btc_sign_multisig_psbt(PtrUR ptr,
                                                  PtrBytes seed,
                                                  uint32_t seed_len,
                                                  PtrBytes master_fingerprint,
                                                  uint32_t master_fingerprint_len);

struct MultisigSignResult *btc_export_multisig_psbt(PtrUR ptr);

struct MultisigSignResult *btc_export_multisig_psbt_bytes(PtrBytes psbt_bytes,
                                                          uint32_t psbt_bytes_length);

PtrT_TransactionCheckResult btc_check_psbt(PtrUR ptr,
                                           PtrBytes master_fingerprint,
                                           uint32_t length,
                                           PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                           PtrString verify_code,
                                           PtrString multisig_wallet_config);

PtrT_TransactionCheckResult btc_check_psbt_bytes(PtrBytes psbt_bytes,
                                                 uint32_t psbt_bytes_length,
                                                 PtrBytes master_fingerprint,
                                                 uint32_t length,
                                                 PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                                 PtrString verify_code,
                                                 PtrString multisig_wallet_config);

struct TransactionParseResult_DisplayTx *btc_parse_psbt_bytes(PtrBytes psbt_bytes,
                                                              uint32_t psbt_bytes_length,
                                                              PtrBytes master_fingerprint,
                                                              uint32_t length,
                                                              PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                                              PtrString multisig_wallet_config);

struct MultisigSignResult *btc_sign_multisig_psbt_bytes(PtrBytes psbt_bytes,
                                                        uint32_t psbt_bytes_length,
                                                        PtrBytes seed,
                                                        uint32_t seed_len,
                                                        PtrBytes master_fingerprint,
                                                        uint32_t master_fingerprint_len);

void free_Response_PsbtSignResult(PtrT_Response_PsbtSignResult ptr);

void free_TransactionParseResult_DisplayTx(PtrT_TransactionParseResult_DisplayTx ptr);

void free_TransactionParseResult_DisplayBtcMsg(PtrT_TransactionParseResult_DisplayBtcMsg ptr);

Ptr_SimpleResponse_c_char cardano_catalyst_xpub(PtrUR ptr);

PtrT_TransactionCheckResult cardano_check_catalyst(PtrUR ptr, PtrBytes master_fingerprint);

PtrT_TransactionCheckResult cardano_check_catalyst_path_type(PtrUR ptr, PtrString cardano_xpub);

Ptr_SimpleResponse_c_char cardano_get_catalyst_root_index(PtrUR ptr);

Ptr_SimpleResponse_c_char cardano_get_sign_data_root_index(PtrUR ptr);

PtrT_TransactionCheckResult cardano_check_sign_data_path_type(PtrUR ptr, PtrString cardano_xpub);

PtrT_TransactionCheckResult cardano_check_sign_data(PtrUR ptr, PtrBytes master_fingerprint);

PtrT_TransactionCheckResult cardano_check_tx(PtrUR ptr,
                                             PtrBytes master_fingerprint,
                                             PtrString cardano_xpub);

PtrT_TransactionCheckResult cardano_check_tx_hash(PtrUR ptr, PtrBytes master_fingerprint);

PtrT_TransactionParseResult_DisplayCardanoSignTxHash cardano_parse_sign_tx_hash(PtrUR ptr);

Ptr_SimpleResponse_c_char cardano_get_path(PtrUR ptr);

PtrT_TransactionParseResult_DisplayCardanoSignData cardano_parse_sign_data(PtrUR ptr);

PtrT_TransactionParseResult_DisplayCardanoCatalyst cardano_parse_catalyst(PtrUR ptr);

PtrT_TransactionParseResult_DisplayCardanoTx cardano_parse_tx(PtrUR ptr,
                                                              PtrBytes master_fingerprint,
                                                              PtrString cardano_xpub);

PtrT_UREncodeResult cardano_sign_catalyst_with_ledger_bitbox02(PtrUR ptr,
                                                               PtrString mnemonic,
                                                               PtrString passphrase);

PtrT_UREncodeResult cardano_sign_catalyst(PtrUR ptr,
                                          PtrBytes entropy,
                                          uint32_t entropy_len,
                                          PtrString passphrase);

PtrT_UREncodeResult cardano_sign_sign_data_with_ledger_bitbox02(PtrUR ptr,
                                                                PtrString mnemonic,
                                                                PtrString passphrase);

PtrT_UREncodeResult cardano_sign_sign_data(PtrUR ptr,
                                           PtrBytes entropy,
                                           uint32_t entropy_len,
                                           PtrString passphrase);

PtrT_UREncodeResult cardano_sign_tx_with_ledger_bitbox02(PtrUR ptr,
                                                         PtrBytes master_fingerprint,
                                                         PtrString cardano_xpub,
                                                         PtrString mnemonic,
                                                         PtrString passphrase,
                                                         bool enable_blind_sign);

PtrT_UREncodeResult cardano_sign_tx(PtrUR ptr,
                                    PtrBytes master_fingerprint,
                                    PtrString cardano_xpub,
                                    PtrBytes entropy,
                                    uint32_t entropy_len,
                                    PtrString passphrase,
                                    bool enable_blind_sign);

struct SimpleResponse_c_char *cardano_get_base_address(PtrString xpub,
                                                       uint32_t index,
                                                       uint8_t network_id);

struct SimpleResponse_c_char *cardano_get_enterprise_address(PtrString xpub,
                                                             uint32_t index,
                                                             uint8_t network_id);

struct SimpleResponse_c_char *cardano_get_stake_address(PtrString xpub,
                                                        uint32_t index,
                                                        uint8_t network_id);

void free_TransactionParseResult_DisplayCardanoTx(PtrT_TransactionParseResult_DisplayCardanoTx ptr);

void free_TransactionParseResult_DisplayCardanoCatalyst(PtrT_TransactionParseResult_DisplayCardanoCatalyst ptr);

void free_TransactionParseResult_DisplayCardanoSignData(PtrT_TransactionParseResult_DisplayCardanoSignData ptr);

void free_TransactionParseResult_DisplayCardanoSignTxHash(PtrT_TransactionParseResult_DisplayCardanoSignTxHash ptr);

PtrT_TransactionCheckResult cosmos_check_tx(PtrUR ptr,
                                            enum QRCodeType ur_type,
                                            PtrBytes master_fingerprint,
                                            uint32_t length);

struct SimpleResponse_c_char *cosmos_get_address(PtrString hd_path,
                                                 PtrString root_x_pub,
                                                 PtrString root_path,
                                                 PtrString prefix);

PtrT_UREncodeResult cosmos_sign_tx(PtrUR ptr,
                                   enum QRCodeType ur_type,
                                   PtrBytes seed,
                                   uint32_t seed_len);

PtrT_TransactionParseResult_DisplayCosmosTx cosmos_parse_tx(PtrUR ptr, enum QRCodeType ur_type);

void free_TransactionParseResult_DisplayCosmosTx(PtrT_TransactionParseResult_DisplayCosmosTx ptr);

PtrT_TransactionCheckResult eth_check_ur_bytes(PtrUR ptr,
                                               PtrBytes master_fingerprint,
                                               uint32_t length,
                                               enum QRCodeType ur_type);

PtrT_TransactionCheckResult eth_check(PtrUR ptr, PtrBytes master_fingerprint, uint32_t length);

PtrString eth_get_root_path_bytes(PtrUR ptr);

PtrString eth_get_root_path(PtrUR ptr);

PtrT_TransactionParseResult_DisplayETH eth_parse_bytes_data(PtrUR ptr, PtrString xpub);

PtrT_TransactionParseResult_DisplayETH eth_parse(PtrUR ptr, PtrString xpub);

PtrT_TransactionParseResult_DisplayETHPersonalMessage eth_parse_personal_message(PtrUR ptr,
                                                                                 PtrString xpub);

PtrT_TransactionParseResult_DisplayETHTypedData eth_parse_typed_data(PtrUR ptr, PtrString xpub);

PtrT_UREncodeResult eth_sign_tx_dynamic(PtrUR ptr,
                                        PtrBytes seed,
                                        uint32_t seed_len,
                                        uintptr_t fragment_length);

PtrT_UREncodeResult eth_sign_tx_bytes(PtrUR ptr,
                                      PtrBytes seed,
                                      uint32_t seed_len,
                                      PtrBytes mfp,
                                      uint32_t mfp_len);

PtrT_UREncodeResult eth_sign_tx(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

PtrT_UREncodeResult eth_sign_tx_unlimited(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

PtrT_TransactionParseResult_EthParsedErc20Transaction eth_parse_erc20(PtrString input,
                                                                      uint32_t decimal);

Ptr_Response_DisplayContractData eth_parse_contract_data(PtrString input_data,
                                                         PtrString contract_json);

Ptr_Response_DisplayContractData eth_parse_contract_data_by_method(PtrString input_data,
                                                                   PtrString contract_name,
                                                                   PtrString contract_method_json);

struct SimpleResponse_c_char *eth_get_address(PtrString hd_path,
                                              PtrString root_x_pub,
                                              PtrString root_path);

void free_TransactionParseResult_DisplayETH(PtrT_TransactionParseResult_DisplayETH ptr);

void free_TransactionParseResult_DisplayETHPersonalMessage(PtrT_TransactionParseResult_DisplayETHPersonalMessage ptr);

void free_TransactionParseResult_DisplayETHTypedData(PtrT_TransactionParseResult_DisplayETHTypedData ptr);

void free_Response_DisplayContractData(PtrT_Response_DisplayContractData ptr);

void free_TransactionParseResult_EthParsedErc20Transaction(PtrT_TransactionParseResult_EthParsedErc20Transaction ptr);

PtrT_TransactionCheckResult near_check(PtrUR ptr, PtrBytes master_fingerprint, uint32_t length);

PtrT_TransactionParseResult_DisplayNearTx near_parse_tx(PtrUR ptr);

PtrT_UREncodeResult near_sign_tx(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

void free_TransactionParseResult_DisplayNearTx(PtrT_TransactionParseResult_DisplayNearTx ptr);

struct SimpleResponse_c_char *solana_get_address(PtrString pubkey);

PtrT_TransactionCheckResult solana_check(PtrUR ptr, PtrBytes master_fingerprint, uint32_t length);

PtrT_TransactionParseResult_DisplaySolanaTx solana_parse_tx(PtrUR ptr);

PtrT_UREncodeResult solana_sign_tx(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

PtrT_TransactionParseResult_DisplaySolanaMessage solana_parse_message(PtrUR ptr, PtrString pubkey);

PtrString sol_get_path(PtrUR ptr);

void free_TransactionParseResult_DisplaySolanaTx(PtrT_TransactionParseResult_DisplaySolanaTx ptr);

void free_TransactionParseResult_DisplaySolanaMessage(PtrT_TransactionParseResult_DisplaySolanaMessage ptr);

struct SimpleResponse_c_char *stellar_get_address(PtrString pubkey);

PtrT_TransactionParseResult_DisplayStellarTx stellar_parse(PtrUR ptr);

PtrT_TransactionCheckResult stellar_check_tx(PtrUR ptr,
                                             PtrBytes master_fingerprint,
                                             uint32_t length);

PtrT_UREncodeResult stellar_sign(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

void free_TransactionParseResult_DisplayStellarTx(PtrT_TransactionParseResult_DisplayStellarTx ptr);

PtrT_TransactionCheckResult sui_check_request(PtrUR ptr,
                                              PtrBytes master_fingerprint,
                                              uint32_t length);

PtrT_TransactionCheckResult sui_check_sign_hash_request(PtrUR ptr,
                                                        PtrBytes master_fingerprint,
                                                        uint32_t length);

struct SimpleResponse_c_char *sui_generate_address(PtrString pub_key);

PtrT_TransactionParseResult_DisplaySuiIntentMessage sui_parse_intent(PtrUR ptr);

PtrT_TransactionParseResult_DisplaySuiSignMessageHash sui_parse_sign_message_hash(PtrUR ptr);

PtrT_UREncodeResult sui_sign_hash(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

PtrT_UREncodeResult sui_sign_intent(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

void free_TransactionParseResult_DisplaySuiIntentMessage(PtrT_TransactionParseResult_DisplaySuiIntentMessage ptr);

void free_TransactionParseResult_DisplaySuiSignMessageHash(PtrT_TransactionParseResult_DisplaySuiSignMessageHash ptr);

PtrT_TransactionParseResult_DisplayTonTransaction ton_parse_transaction(PtrUR ptr);

PtrT_TransactionParseResult_DisplayTonProof ton_parse_proof(PtrUR ptr);

PtrT_TransactionCheckResult ton_check_transaction(PtrUR ptr, PtrString public_key);

PtrT_TransactionCheckResult ton_not_supported_error(void);

PtrT_UREncodeResult ton_sign_transaction(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

PtrT_UREncodeResult ton_sign_proof(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

bool ton_verify_mnemonic(PtrString mnemonic);

Ptr_VecFFI_u8 ton_mnemonic_to_entropy(PtrString mnemonic);

struct SimpleResponse_u8 *ton_entropy_to_seed(PtrBytes entropy, uint32_t entropy_len);

struct SimpleResponse_u8 *ton_mnemonic_to_seed(PtrString mnemonic);

struct SimpleResponse_c_char *ton_seed_to_publickey(PtrBytes seed, uint32_t seed_len);

struct SimpleResponse_c_char *ton_get_address(PtrString public_key);

void free_TransactionParseResult_DisplayTonTransaction(PtrT_TransactionParseResult_DisplayTonTransaction ptr);

void free_TransactionParseResult_DisplayTonProof(PtrT_TransactionParseResult_DisplayTonProof ptr);

PtrT_TransactionCheckResult tron_check_keystone(PtrUR ptr,
                                                enum QRCodeType ur_type,
                                                PtrBytes master_fingerprint,
                                                uint32_t length,
                                                PtrString x_pub);

struct TransactionParseResult_DisplayTron *tron_parse_keystone(PtrUR ptr,
                                                               enum QRCodeType ur_type,
                                                               PtrBytes master_fingerprint,
                                                               uint32_t length,
                                                               PtrString x_pub);

struct UREncodeResult *tron_sign_keystone(PtrUR ptr,
                                          enum QRCodeType ur_type,
                                          PtrBytes master_fingerprint,
                                          uint32_t length,
                                          PtrString x_pub,
                                          int32_t cold_version,
                                          PtrBytes seed,
                                          uint32_t seed_len);

struct SimpleResponse_c_char *tron_get_address(PtrString hd_path, PtrString x_pub);

void free_TransactionParseResult_DisplayTron(PtrT_TransactionParseResult_DisplayTron ptr);

struct SimpleResponse_c_char *xrp_get_address(PtrString hd_path,
                                              PtrString root_x_pub,
                                              PtrString root_path);

PtrT_TransactionParseResult_DisplayXrpTx xrp_parse_tx(PtrUR ptr);

PtrT_UREncodeResult xrp_sign_tx_bytes(PtrUR ptr,
                                      PtrBytes seed,
                                      uint32_t seed_len,
                                      PtrBytes mfp,
                                      uint32_t mfp_len,
                                      PtrString root_xpub);

PtrT_UREncodeResult xrp_sign_tx(PtrUR ptr, PtrString hd_path, PtrBytes seed, uint32_t seed_len);

PtrT_TransactionCheckResult xrp_check_tx(PtrUR ptr, PtrString root_xpub, PtrString cached_pubkey);

bool is_keystone_xrp_tx(PtrUR ur_data_ptr);

PtrT_TransactionCheckResult xrp_check_tx_bytes(PtrUR ptr,
                                               PtrBytes master_fingerprint,
                                               uint32_t length,
                                               enum QRCodeType ur_type);

PtrT_TransactionParseResult_DisplayXrpTx xrp_parse_bytes_tx(PtrUR ptr);

void free_TransactionParseResult_DisplayXrpTx(PtrT_TransactionParseResult_DisplayXrpTx ptr);

struct SimpleResponse_c_char *derive_zcash_ufvk(PtrBytes seed,
                                                uint32_t seed_len,
                                                PtrString account_path);

struct SimpleResponse_u8 *calculate_zcash_seed_fingerprint(PtrBytes seed, uint32_t seed_len);

struct SimpleResponse_c_char *generate_zcash_default_address(PtrString ufvk_text);

struct TransactionCheckResult *check_zcash_tx(PtrUR tx,
                                              PtrString ufvk,
                                              PtrBytes seed_fingerprint,
                                              uint32_t account_index,
                                              bool disabled);

Ptr_TransactionParseResult_DisplayPczt parse_zcash_tx(PtrUR tx,
                                                      PtrString ufvk,
                                                      PtrBytes seed_fingerprint);

struct UREncodeResult *sign_zcash_tx(PtrUR tx, PtrBytes seed, uint32_t seed_len);

void free_TransactionParseResult_DisplayPczt(PtrT_TransactionParseResult_DisplayPczt ptr);

#endif  /* _LIBRUST_C_H */

#ifndef _LIBRUST_C_H
#define _LIBRUST_C_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#define BUILD_MULTI_COINS

typedef enum ETHAccountType {
  Bip44Standard,
  LedgerLive,
  LedgerLegacy,
} ETHAccountType;

typedef enum URType {
  CryptoPSBT,
  CryptoMultiAccounts,
  Bytes,
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
  CosmosSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  EvmSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  SuiSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  AptosSignRequest,
#endif
#if defined(BUILD_MULTI_COINS)
  QRHardwareCall,
#endif
  URTypeUnKnown,
} URType;

typedef enum ViewType {
  BtcNativeSegwitTx,
  BtcSegwitTx,
  BtcLegacyTx,
  BtcTx,
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
  AptosTx,
#endif
  WebAuthResult,
#if defined(BUILD_MULTI_COINS)
  KeyDerivationRequest,
#endif
  ViewTypeUnKnown,
} ViewType;

typedef char *PtrString;

typedef struct SimpleResponse_c_char {
  char *data;
  uint32_t error_code;
  PtrString error_message;
} SimpleResponse_c_char;

typedef struct TransactionCheckResult {
  uint32_t error_code;
  PtrString error_message;
} TransactionCheckResult;

typedef struct TransactionCheckResult *PtrT_TransactionCheckResult;

typedef void *PtrVoid;

typedef PtrVoid PtrUR;

typedef uint8_t *PtrBytes;

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

typedef PtrVoid PtrEncoder;

typedef struct UREncodeResult {
  bool is_multi_part;
  char *data;
  PtrEncoder encoder;
  uint32_t error_code;
  char *error_message;
} UREncodeResult;

typedef struct UREncodeResult *PtrT_UREncodeResult;

typedef struct SimpleResponse_u8 {
  uint8_t *data;
  uint32_t error_code;
  PtrString error_message;
} SimpleResponse_u8;

typedef PtrVoid PtrDecoder;

typedef struct URParseResult {
  bool is_multi_part;
  uint32_t progress;
  enum ViewType t;
  enum URType ur_type;
  PtrUR data;
  PtrDecoder decoder;
  uint32_t error_code;
  PtrString error_message;
} URParseResult;

typedef struct URParseResult *PtrT_URParseResult;

typedef struct URParseMultiResult {
  bool is_complete;
  enum ViewType t;
  enum URType ur_type;
  uint32_t progress;
  PtrUR data;
  uint32_t error_code;
  PtrString error_message;
} URParseMultiResult;

typedef struct URParseMultiResult *PtrT_URParseMultiResult;

typedef struct UREncodeMultiResult {
  char *data;
  uint32_t error_code;
  char *error_message;
} UREncodeMultiResult;

typedef struct UREncodeMultiResult *PtrT_UREncodeMultiResult;

typedef struct SimpleResponse_u8 *PtrT_SimpleResponse_u8;

typedef struct SimpleResponse_c_char *PtrT_SimpleResponse_c_char;

typedef const void *ConstPtrVoid;

typedef ConstPtrVoid ConstPtrUR;

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
  bool fee_larger_than_amount;
} DisplayTxOverview;

typedef struct DisplayTxDetailInput {
  bool has_address;
  PtrString address;
  PtrString amount;
  bool is_mine;
  PtrString path;
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

typedef struct TransactionParseResult_DisplayTx *PtrT_TransactionParseResult_DisplayTx;

typedef struct SimpleResponse_c_char *Ptr_SimpleResponse_c_char;

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

typedef struct DisplayCardanoCertificate {
  PtrString cert_type;
  PtrString address;
  PtrString pool;
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

typedef struct DisplaySolanaTxOverview {
  PtrString display_type;
  PtrString main_action;
  PtrString transfer_value;
  PtrString transfer_from;
  PtrString transfer_to;
  PtrT_VecFFI_DisplaySolanaTxOverviewVotesOn votes_on;
  PtrString vote_account;
  PtrT_VecFFI_DisplaySolanaTxOverviewGeneral general;
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

typedef struct DisplaySuiIntentMessage {
  PtrString detail;
} DisplaySuiIntentMessage;

typedef struct TransactionParseResult_DisplaySuiIntentMessage {
  struct DisplaySuiIntentMessage *data;
  uint32_t error_code;
  PtrString error_message;
} TransactionParseResult_DisplaySuiIntentMessage;

typedef struct TransactionParseResult_DisplaySuiIntentMessage *PtrT_TransactionParseResult_DisplaySuiIntentMessage;

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

typedef struct KeyDerivationSchema {
  PtrString key_path;
  PtrString curve;
  PtrString algo;
} KeyDerivationSchema;

typedef struct KeyDerivationSchema *PtrT_KeyDerivationSchema;

typedef struct VecFFI_KeyDerivationSchema {
  PtrT_KeyDerivationSchema data;
  size_t size;
  size_t cap;
} VecFFI_KeyDerivationSchema;

typedef struct VecFFI_KeyDerivationSchema *Ptr_VecFFI_KeyDerivationSchema;

typedef struct KeyDerivationRequestData {
  Ptr_VecFFI_KeyDerivationSchema schemas;
} KeyDerivationRequestData;

typedef struct KeyDerivationRequestData *Ptr_KeyDerivationRequestData;

typedef struct QRHardwareCallData {
  PtrString call_type;
  PtrString origin;
  Ptr_KeyDerivationRequestData key_derivation;
} QRHardwareCallData;

typedef struct Response_QRHardwareCallData {
  struct QRHardwareCallData *data;
  uint32_t error_code;
  PtrString error_message;
} Response_QRHardwareCallData;

typedef struct Response_QRHardwareCallData *Ptr_Response_QRHardwareCallData;

typedef struct KeplrAccount {
  PtrString name;
  PtrString path;
  PtrString xpub;
} KeplrAccount;

typedef struct KeplrAccount *PtrT_KeplrAccount;

typedef struct CSliceFFI_KeplrAccount {
  PtrT_KeplrAccount data;
  size_t size;
} CSliceFFI_KeplrAccount;

typedef struct CSliceFFI_KeplrAccount *PtrT_CSliceFFI_KeplrAccount;

typedef struct Response_QRHardwareCallData *PtrT_Response_QRHardwareCallData;

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

extern const uintptr_t FRAGMENT_MAX_LENGTH_DEFAULT;

struct SimpleResponse_c_char *aptos_generate_address(PtrString pub_key);

PtrT_TransactionCheckResult aptos_check_request(PtrUR ptr,
                                                PtrBytes master_fingerprint,
                                                uint32_t length);

PtrT_TransactionParseResult_DisplayAptosTx aptos_parse(PtrUR ptr);

PtrT_UREncodeResult aptos_sign_tx(PtrUR ptr, PtrBytes seed, uint32_t seed_len, PtrString pub_key);

PtrString aptos_get_path(PtrUR ptr);

struct SimpleResponse_c_char *test_aptos_parse(void);

void free_TransactionParseResult_DisplayAptosTx(PtrT_TransactionParseResult_DisplayAptosTx ptr);

struct SimpleResponse_u8 *get_master_fingerprint(PtrBytes seed, uint32_t seed_len);

struct SimpleResponse_c_char *get_extended_pubkey_by_seed(PtrBytes seed,
                                                          uint32_t seed_len,
                                                          PtrString path);

struct SimpleResponse_c_char *get_ed25519_pubkey_by_seed(PtrBytes seed,
                                                         uint32_t seed_len,
                                                         PtrString path);

struct SimpleResponse_c_char *get_bip32_ed25519_extended_pubkey(PtrBytes entropy,
                                                                uint32_t entropy_len,
                                                                PtrString passphrase,
                                                                PtrString path);

struct SimpleResponse_c_char *get_icarus_master_key(PtrBytes entropy,
                                                    uint32_t entropy_len,
                                                    PtrString passphrase);

struct SimpleResponse_c_char *derive_bip32_ed25519_extended_pubkey(PtrString master_key,
                                                                   PtrString path);

struct SimpleResponse_c_char *k1_sign_message_hash_by_private_key(PtrBytes private_key,
                                                                  PtrBytes message_hash);

bool k1_verify_signature(PtrBytes signature, PtrBytes message_hash, PtrBytes public_key);

struct SimpleResponse_u8 *pbkdf2_rust(PtrBytes password, PtrBytes salt, uint32_t iterations);

void free_ur_parse_result(PtrT_URParseResult ur_parse_result);

void free_ur_parse_multi_result(PtrT_URParseMultiResult ptr);

void free_ur_encode_result(PtrT_UREncodeResult ptr);

void free_ur_encode_muilt_result(PtrT_UREncodeMultiResult ptr);

void free_simple_response_u8(PtrT_SimpleResponse_u8 ptr);

void free_simple_response_c_char(PtrT_SimpleResponse_c_char ptr);

void free_ptr_string(PtrString ptr);

void free_rust_value(void *any_ptr);

void free_TransactionCheckResult(PtrT_TransactionCheckResult ptr);

struct UREncodeMultiResult *get_next_part(PtrEncoder ptr);

struct URParseResult *parse_ur(PtrString ur);

struct URParseMultiResult *receive(PtrString ur, PtrDecoder decoder);

PtrString calculate_auth_code(ConstPtrUR web_auth_data,
                              PtrBytes rsa_key_n,
                              uint32_t rsa_key_n_len,
                              PtrBytes rsa_key_d,
                              uint32_t rsa_key_d_len);

struct SimpleResponse_c_char *utxo_get_address(PtrString hd_path, PtrString x_pub);

struct SimpleResponse_c_char *xpub_convert_version(PtrString x_pub, PtrString target);

struct TransactionParseResult_DisplayTx *utxo_parse_keystone(PtrUR ptr,
                                                             enum URType ur_type,
                                                             PtrBytes master_fingerprint,
                                                             uint32_t length,
                                                             PtrString x_pub);

struct UREncodeResult *utxo_sign_keystone(PtrUR ptr,
                                          enum URType ur_type,
                                          PtrBytes master_fingerprint,
                                          uint32_t length,
                                          PtrString x_pub,
                                          int32_t cold_version,
                                          PtrBytes seed,
                                          uint32_t seed_len);

PtrT_TransactionCheckResult utxo_check_keystone(PtrUR ptr,
                                                enum URType ur_type,
                                                PtrBytes master_fingerprint,
                                                uint32_t length,
                                                PtrString x_pub);

struct TransactionParseResult_DisplayTx *btc_parse_psbt(PtrUR ptr,
                                                        PtrBytes master_fingerprint,
                                                        uint32_t length,
                                                        PtrT_CSliceFFI_ExtendedPublicKey public_keys);

struct UREncodeResult *btc_sign_psbt(PtrUR ptr,
                                     PtrBytes seed,
                                     uint32_t seed_len,
                                     PtrBytes master_fingerprint,
                                     uint32_t master_fingerprint_len);

PtrT_TransactionCheckResult btc_check_psbt(PtrUR ptr,
                                           PtrBytes master_fingerprint,
                                           uint32_t length,
                                           PtrT_CSliceFFI_ExtendedPublicKey public_keys);

void free_TransactionParseResult_DisplayTx(PtrT_TransactionParseResult_DisplayTx ptr);

PtrT_TransactionCheckResult cardano_check_tx(PtrUR ptr,
                                             PtrBytes master_fingerprint,
                                             PtrString cardano_xpub);

Ptr_SimpleResponse_c_char cardano_get_path(PtrUR ptr);

PtrT_TransactionParseResult_DisplayCardanoTx cardano_parse_tx(PtrUR ptr,
                                                              PtrBytes master_fingerprint,
                                                              PtrString cardano_xpub);

PtrT_UREncodeResult cardano_sign_tx(PtrUR ptr,
                                    PtrBytes master_fingerprint,
                                    PtrString cardano_xpub,
                                    PtrBytes entropy,
                                    uint32_t entropy_len,
                                    PtrString passphrase);

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

PtrT_TransactionCheckResult cosmos_check_tx(PtrUR ptr,
                                            enum URType ur_type,
                                            PtrBytes master_fingerprint,
                                            uint32_t length);

struct SimpleResponse_c_char *cosmos_get_address(PtrString hd_path,
                                                 PtrString root_x_pub,
                                                 PtrString root_path,
                                                 PtrString prefix);

PtrT_UREncodeResult cosmos_sign_tx(PtrUR ptr,
                                   enum URType ur_type,
                                   PtrBytes seed,
                                   uint32_t seed_len);

PtrT_TransactionParseResult_DisplayCosmosTx cosmos_parse_tx(PtrUR ptr, enum URType ur_type);

void free_TransactionParseResult_DisplayCosmosTx(PtrT_TransactionParseResult_DisplayCosmosTx ptr);

PtrT_TransactionCheckResult eth_check(PtrUR ptr, PtrBytes master_fingerprint, uint32_t length);

PtrString eth_get_root_path(PtrUR ptr);

PtrT_TransactionParseResult_DisplayETH eth_parse(PtrUR ptr, PtrString xpub);

PtrT_TransactionParseResult_DisplayETHPersonalMessage eth_parse_personal_message(PtrUR ptr,
                                                                                 PtrString xpub);

PtrT_TransactionParseResult_DisplayETHTypedData eth_parse_typed_data(PtrUR ptr, PtrString xpub);

PtrT_UREncodeResult eth_sign_tx_dynamic(PtrUR ptr,
                                        PtrBytes seed,
                                        uint32_t seed_len,
                                        uintptr_t fragment_length);

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

PtrT_TransactionCheckResult sui_check_request(PtrUR ptr,
                                              PtrBytes master_fingerprint,
                                              uint32_t length);

struct SimpleResponse_c_char *sui_generate_address(PtrString pub_key);

PtrT_TransactionParseResult_DisplaySuiIntentMessage sui_parse_intent(PtrUR ptr);

PtrT_UREncodeResult sui_sign_intent(PtrUR ptr, PtrBytes seed, uint32_t seed_len);

void free_TransactionParseResult_DisplaySuiIntentMessage(PtrT_TransactionParseResult_DisplaySuiIntentMessage ptr);

struct URParseResult *test_get_crypto_psbt(void);

struct URParseResult *test_get_btc_keystone_bytes(void);

struct UREncodeResult *test_encode_crypto_psbt(void);

struct URParseResult *test_decode_crypto_psbt(void);

struct URParseResult *test_decode_btc_keystone_sign_result(void);

struct URParseResult *test_decode_keystone_sign_request(void);

struct URParseResult *test_decode_crypto_psbt_1(void);

struct URParseMultiResult *test_decode_crypto_psbt_2(PtrDecoder decoder);

struct URParseMultiResult *test_decode_crypto_psbt_3(PtrDecoder decoder);

struct UREncodeResult *test_connect_blue_wallet(void);

struct UREncodeResult *get_connect_blue_wallet_ur(uint8_t *master_fingerprint,
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

struct UREncodeResult *get_connect_metamask_ur_dynamic(PtrBytes master_fingerprint,
                                                       uint32_t master_fingerprint_length,
                                                       enum ETHAccountType account_type,
                                                       PtrT_CSliceFFI_ExtendedPublicKey public_keys,
                                                       uintptr_t fragment_max_length_default,
                                                       uintptr_t fragment_max_length_other);

struct UREncodeResult *get_connect_metamask_ur_unlimited(PtrBytes master_fingerprint,
                                                         uint32_t master_fingerprint_length,
                                                         enum ETHAccountType account_type,
                                                         PtrT_CSliceFFI_ExtendedPublicKey public_keys);

struct UREncodeResult *get_connect_metamask_ur(PtrBytes master_fingerprint,
                                               uint32_t master_fingerprint_length,
                                               enum ETHAccountType account_type,
                                               PtrT_CSliceFFI_ExtendedPublicKey public_keys);

Ptr_Response_QRHardwareCallData parse_qr_hardware_call(PtrUR ur);

Ptr_UREncodeResult generate_key_derivation_ur(PtrBytes master_fingerprint,
                                              uint32_t master_fingerprint_length,
                                              Ptr_CSliceFFI_ExtendedPublicKey xpubs);

struct UREncodeResult *get_connect_aptos_wallet_ur(uint8_t *master_fingerprint,
                                                   uint32_t length,
                                                   PtrT_CSliceFFI_ExtendedPublicKey public_keys);

Ptr_UREncodeResult get_connect_imtoken_ur(PtrBytes master_fingerprint,
                                          uint32_t master_fingerprint_length,
                                          PtrString xpub,
                                          PtrString wallet_name);

struct UREncodeResult *get_connect_keplr_wallet_ur(PtrBytes master_fingerprint,
                                                   uint32_t master_fingerprint_length,
                                                   PtrT_CSliceFFI_KeplrAccount keplr_accounts);

Ptr_UREncodeResult get_okx_wallet_ur(PtrBytes master_fingerprint,
                                     uint32_t master_fingerprint_length,
                                     PtrString serial_number,
                                     Ptr_CSliceFFI_ExtendedPublicKey public_keys,
                                     PtrString device_type,
                                     PtrString device_version);

struct UREncodeResult *get_connect_solana_wallet_ur(uint8_t *master_fingerprint,
                                                    uint32_t length,
                                                    PtrT_CSliceFFI_ExtendedPublicKey public_keys);

void free_Response_QRHardwareCallData(PtrT_Response_QRHardwareCallData ptr);

struct UREncodeResult *get_connect_sui_wallet_ur(uint8_t *master_fingerprint,
                                                 uint32_t length,
                                                 PtrT_CSliceFFI_ExtendedPublicKey public_keys);

struct UREncodeResult *get_connect_xrp_toolkit_ur(PtrString hd_path,
                                                  PtrString root_x_pub,
                                                  PtrString root_path);

struct URParseResult *test_get_bch_keystone_succeed_bytes(void);

struct URParseResult *test_get_ltc_keystone_bytes(void);

struct URParseResult *test_get_dash_keystone_bytes(void);

struct URParseResult *test_get_bch_keystone_bytes(void);

struct URParseResult *test_get_tron_keystone_bytes(void);

struct URParseResult *test_get_tron_check_failed_keystone_bytes(void);

struct URParseResult *test_get_eth_sign_request(void);

struct URParseResult *test_get_eth_sign_request_for_personal_message(void);

struct URParseResult *test_get_sol_sign_request(char *cbor);

struct URParseResult *test_get_sol_sign_message(char *cbor);

struct URParseResult *test_get_sui_sign_request(char *cbor);

struct URParseResult *test_get_aptos_sign_request(char *cbor);

struct URParseResult *test_get_near_sign_request(char *cbor);

struct URParseResult *test_get_eth_eip1559_sign_request(void);

struct URParseResult *test_get_cardano_sign_request(void);

struct URParseResult *test_get_xrp_sign_request(void);

struct URParseResult *test_get_xrp_parse_request(char *cbor);

struct URParseResult *test_get_cosmos_sign_request(char *cbor);

struct URParseResult *test_get_cosmos_evm_sign_request(char *cbor);

PtrT_TransactionCheckResult tron_check_keystone(PtrUR ptr,
                                                enum URType ur_type,
                                                PtrBytes master_fingerprint,
                                                uint32_t length,
                                                PtrString x_pub);

struct TransactionParseResult_DisplayTron *tron_parse_keystone(PtrUR ptr,
                                                               enum URType ur_type,
                                                               PtrBytes master_fingerprint,
                                                               uint32_t length,
                                                               PtrString x_pub);

struct UREncodeResult *tron_sign_keystone(PtrUR ptr,
                                          enum URType ur_type,
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

PtrT_UREncodeResult xrp_sign_tx(PtrUR ptr, PtrString hd_path, PtrBytes seed, uint32_t seed_len);

PtrT_TransactionCheckResult xrp_check_tx(PtrUR ptr, PtrString root_xpub, PtrString cached_pubkey);

void free_TransactionParseResult_DisplayXrpTx(PtrT_TransactionParseResult_DisplayXrpTx ptr);

#endif /* _LIBRUST_C_H */

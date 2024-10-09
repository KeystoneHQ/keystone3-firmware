use alloc::{
    string::String,
    vec::{Vec},
};
use app_utils::impl_public_struct;

use crate::{
    data_item::{DataItem, Tag},
    errors::ArweaveError,
    tokens::find_token,
};

impl_public_struct!(AOTransferTransaction {
    from: String,
    to: String,
    quantity: String,
    token_id: String,
    other_info: Vec<Tag>
});

impl TryFrom<DataItem> for AOTransferTransaction {
    type Error = ArweaveError;

    fn try_from(value: DataItem) -> Result<Self, Self::Error> {
        let tags = value.get_tags().get_data();
        let protocol = tags
            .iter()
            .find(|i| i.get_name().eq("Data-Protocol") && i.get_value().eq("ao"));
        let action = tags
            .iter()
            .find(|i| i.get_name().eq("Action") && i.get_value().eq("Transfer"));
        let recipient = tags.iter().find(|i| i.get_name().eq("Recipient"));
        let quantity = tags.iter().find(|i| i.get_name().eq("Quantity"));
        let token_id = value.get_target();
        let mut rest_tags = tags.iter().filter(|v| {
            v.get_name().ne("DataProtocol")
                && v.get_name().ne("Action")
                && v.get_name().ne("Recipient")
                && v.get_name().ne("Quantity")
        });
        if let (Some(_action), Some(_protocol), Some(token_id), Some(recipient), Some(quantity)) =
            (action, protocol, token_id, recipient, quantity)
        {
            let from = value.get_owner();
            let to = recipient.get_value();
            let quantity = quantity.get_value();
            let mut tags = vec![];
            loop {
                match rest_tags.next() {
                    Some(tag) => tags.push(tag.clone()),
                    None => break,
                }
            }

            let token_info = find_token(&token_id);
            if let Some(token_info) = token_info {
                if let Ok(amount) = token_info.convert_quantity(&quantity) {
                    return Ok(Self {
                        from,
                        to,
                        quantity: amount,
                        token_id: token_info.get_name(),
                        other_info: tags,
                    });
                }
            }

            Ok(Self {
                from,
                to,
                quantity,
                token_id,
                other_info: tags,
            })
        } else {
            Err(ArweaveError::NotAOTransaction)
        }
    }
}

#[cfg(test)]
mod tests {
    use third_party::hex;

    use crate::data_item::DataItem;

    use super::AOTransferTransaction;

    #[test]
    fn test_transform_ao_transfer() {
        let binary = hex::decode("01000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a999bac8b7906c0bc94f7d163ea9e7fe6ef34045b6a27035e5298aaaddeea05355c50efd30da262c97a68b5aa7219726754bf8501818429e60b9f8175ed66a23349757dc8b3f126abc199272c91174bdb96a9a13aad43b9b6195583188c222002d29b105169dc237dccb0e371895aa10b9263e0b6fbe2d03d3a0073fa7f278ecfa890e75a3fe812ca86eb44f134a7edaa664a5582e72fa43b7accdfeb03f0492c118235b9ff7784106ca1a2f6e7bc4bcc6e1ed98775b7c023a1ae1e332f42e3183ab17c43c58e6605353a47331452ebf659fb267d27492b961ecdafcde9657a0a623aec761f6b3130f89ff7136cae26ebc58aaaa0c6c2264d8e0aa7c78cb46b5210cd69be2ffca64fd3cb0990116034c582828dd22d0235edf9ad999ef0b25afbcab802330d03e9653eff2dbee7f9e0a695a63e04d2aaef73152c255a1d8e5f9cc525cbcfd796ffff337f21d846ae7091037e2bfd06efaf262375100323335e62c79ca63aa31226e3655acab5f2861913630be567210d3d0d5b0f0a6bdc7edfc986e9c14b28b9d32deab5041872a26f8b95341a8cdf6326207d0c2f728ef85554f18c9e285c9f3e01e1d1cb1adf2546eeb9ddfc81a51b0fdf94c9f9116adcd5878815d21038968cbef2b51cc4a27fb1911008c6d1d587830645aca9ca775cf1d67dd9901aadb830a1e8abe0548a47619b8d80083316a645c646820640067653101c54f73164ab75f6650ea8970355bebd6f5162237379174d6afbc4a403e9d875d000800000000000000b100000000000000100c416374696f6e105472616e7366657212526563697069656e745671667a34427465626f714d556f4e536c74457077394b546462663736665252446667783841693644474a77105175616e746974791631303030303030303030301a446174612d50726f746f636f6c04616f0e56617269616e740e616f2e544e2e3108547970650e4d6573736167650653444b12616f636f6e6e65637418436f6e74656e742d5479706514746578742f706c61696e0037373037").unwrap();
        let result = DataItem::deserialize(&binary).unwrap();
        let ao_transfer = AOTransferTransaction::try_from(result).unwrap();
        assert_eq!(
            ao_transfer.from,
            "nSkowCiV4VBZJVyI2UK2wT_6g9LVX5BLZvYSTjd0bVQ"
        );
        assert_eq!(
            ao_transfer.to,
            "qfz4BteboqMUoNSltEpw9KTdbf76fRRDfgx8Ai6DGJw"
        );
        assert_eq!(ao_transfer.quantity, "0.01 AR");
        assert_eq!(ao_transfer.token_id, "Wrapped AR");
    }
}

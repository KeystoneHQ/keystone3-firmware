use crate::errors::{Result, SolanaError};
use alloc::format;
use alloc::string::String;
use bitcoin::base58;

pub fn get_address(pub_key: &String) -> Result<String> {
    let pubkey = hex::decode(pub_key)?;
    if pubkey.len() != 32 {
        SolanaError::AddressError(format!("bad public key {:?}", pub_key));
    }
    Ok(base58::encode(pubkey.as_slice()))
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;

    use super::*;

    #[test]
    fn test_address() {
        {
            {
                // let path0 = "m/44'/501'/0'".to_string();
                // let path1 = "m/44'/501'/1'".to_string();
                // let path2 = "m/44'/501'/2'".to_string();
                assert_eq!(
                    "GjJyeC1r2RgkuoCWMyPYkCWSGSGLcz266EaAkLA27AhL",
                    get_address(
                        &"e9b6062841bb977ad21de71ec961900633c26f21384e015b014a637a61499547"
                            .to_string()
                    )
                    .unwrap()
                );
                assert_eq!(
                    "ANf3TEKFL6jPWjzkndo4CbnNdUNkBk4KHPggJs2nu8Xi",
                    get_address(
                        &"8b4564d4b6be05d6ead16d246c5e30773da9459040370284b57c944a3d0a1481"
                            .to_string()
                    )
                    .unwrap()
                );
                assert_eq!(
                    "Ag74i82rUZBTgMGLacCA1ZLnotvAca8CLscXcrG6Nwem",
                    get_address(
                        &"8fbdb0bf4cfb56b3393d8f41985ce37c0a4bf749715fa8f8f153b6f35590d6de"
                            .to_string()
                    )
                    .unwrap()
                );
            }
            {
                // let path0 = "m/44'/501'".to_string();
                assert_eq!(
                    "D2PPQSYFe83nDzk96FqGumVU8JA7J8vj2Rhjc2oXzEi5",
                    get_address(
                        &"b2a722dc18dd5c49c3f48e9b0726f11be66786e91cac573498d6ee88392cc96a"
                            .to_string()
                    )
                    .unwrap()
                );
            }
            {
                // let path0 = "m/44'/501'/0'/0'".to_string();
                // let path1 = "m/44'/501'/1'/0'".to_string();
                // let path2 = "m/44'/501'/2'/0'".to_string();
                assert_eq!(
                    "HAgk14JpMQLgt6rVgv7cBQFJWFto5Dqxi472uT3DKpqk",
                    get_address(
                        &"f036276246a75b9de3349ed42b15e232f6518fc20f5fcd4f1d64e81f9bd258f7"
                            .to_string()
                    )
                    .unwrap()
                );
                assert_eq!(
                    "Hh8QwFUA6MtVu1qAoq12ucvFHNwCcVTV7hpWjeY1Hztb",
                    get_address(
                        &"f8029acf5cbcbdd5ac46ec147f3b78a3df6e5022ef0411db2bab650d329a4cd4"
                            .to_string()
                    )
                    .unwrap()
                );
                assert_eq!(
                    "7WktogJEd2wQ9eH2oWusmcoFTgeYi6rS632UviTBJ2jm",
                    get_address(
                        &"60c5985f58a32ff8ab91e2fbd1d211b8de6b4acc4f6ce4458830efc0c801ca1c"
                            .to_string()
                    )
                    .unwrap()
                );
            }
        }
    }
}

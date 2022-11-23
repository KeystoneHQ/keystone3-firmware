#[macro_export]
macro_rules! collect {
    ($t: expr) => {{
        $t.iter().map(|each| (*each).clone().try_into()).collect()
    }};
}

#[macro_export]
macro_rules! check_hd_path {
    ($t: expr) => {{
        let mut result: Result<()> = Ok(());
        if $t.len() != 6 {
            result = Err(BitcoinError::SignLegacyTxError(format!(
                "invalid hd_path {:?}",
                $t
            )));
        };
        result
    }};
}

#[macro_export]
macro_rules! derivation_address_path {
    ($t: expr) => {{
        let parts = $t.split("/").collect::<Vec<&str>>();
        let result: Result<String> = match crate::check_hd_path!(parts) {
            Ok(_) => {
                let path = parts.as_slice()[parts.len() - 2..].to_vec().join("/");
                Ok(format!("{}{}", "m/", path))
            }
            Err(e) => Err(e),
        };
        result
    }};
}

#[macro_export]
macro_rules! derivation_account_path {
    ($t: expr) => {{
        let parts = $t.split("/").collect::<Vec<&str>>();
        let result: Result<String> = match crate::check_hd_path!(parts) {
            Ok(_) => {
                let path = parts.as_slice()[1..parts.len() - 2].to_vec().join("/");
                Ok(format!("{}{}", "m/", path))
            }
            Err(e) => Err(e),
        };
        result
    }};
}

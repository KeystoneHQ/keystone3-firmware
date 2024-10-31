use alloc::format;
use alloc::string::String;
use core::ops::Div;
use serde::Serializer;

pub const NEAR_THRESHOLD: u128 = 10000000000000000000;
pub const NEAR_DIVIDER: f64 = 1_000_000_000_000_000_000_000_000f64;
pub const GAS_DIVIDER: f64 = 1000_000_000_000f64;

use serde_json;
use serde_json::Value;

pub trait Merge {
    fn merge(&mut self, new_json_value: Value);
}

impl Merge for serde_json::Value {
    fn merge(&mut self, new_json_value: Value) {
        merge(self, &new_json_value);
    }
}

fn merge(a: &mut Value, b: &Value) {
    match (a, b) {
        (Value::Object(ref mut a), &Value::Object(ref b)) => {
            for (k, v) in b {
                merge(a.entry(k).or_insert(Value::Null), v);
            }
        }
        (Value::Array(ref mut a), &Value::Array(ref b)) => {
            a.extend(b.clone());
        }
        (Value::Array(ref mut a), &Value::Object(ref b)) => {
            a.extend([Value::Object(b.clone())]);
        }
        (a, b) => {
            *a = b.clone();
        }
    }
}

pub fn format_amount(value: u128) -> String {
    return if value > NEAR_THRESHOLD {
        format!("{} {}", (value as f64).div(NEAR_DIVIDER), "NEAR")
    } else {
        format!("{} {}", value, "Yocto")
    };
}

pub fn format_option_u128_amount<S>(v: &Option<u128>, serializer: S) -> Result<S::Ok, S::Error>
where
    S: Serializer,
{
    serializer.serialize_str(&format_amount(v.unwrap_or(u128::default())))
}

pub fn format_u128_amount<S>(v: &u128, serializer: S) -> Result<S::Ok, S::Error>
where
    S: Serializer,
{
    serializer.serialize_str(&format_amount(v.clone()))
}

pub fn format_gas(value: u64) -> String {
    format!("{} {}", (value as f64).div(GAS_DIVIDER), "TGas")
}

pub fn format_gas_amount<S>(v: &u64, serializer: S) -> Result<S::Ok, S::Error>
where
    S: Serializer,
{
    serializer.serialize_str(&format_gas(v.clone()))
}

#[cfg(test)]
mod serde_json_value_updater_test {
    use super::*;
    use alloc::string::ToString;
    use serde_json;
    use serde_json::Value;

    #[test]
    fn it_should_merge_array_string() {
        let mut first_json_value: Value = serde_json::from_str(r#"["a","b"]"#).unwrap();
        let secound_json_value: Value = serde_json::from_str(r#"["b","c"]"#).unwrap();
        first_json_value.merge(secound_json_value);
        assert_eq!(r#"["a","b","b","c"]"#, first_json_value.to_string());
    }

    #[test]
    fn it_should_merge_array_object() {
        let mut first_json_value: Value =
            serde_json::from_str(r#"[{"value":"a"},{"value":"b"}]"#).unwrap();
        let secound_json_value: Value =
            serde_json::from_str(r#"[{"value":"b"},{"value":"c"}]"#).unwrap();
        first_json_value.merge(secound_json_value);
        assert_eq!(
            r#"[{"value":"a"},{"value":"b"},{"value":"b"},{"value":"c"}]"#,
            first_json_value.to_string()
        );
    }

    #[test]
    fn it_should_merge_object() {
        let mut first_json_value: Value =
            serde_json::from_str(r#"{"value1":"a","value2":"b"}"#).unwrap();
        let secound_json_value: Value =
            serde_json::from_str(r#"{"value1":"a","value2":"c","value3":"d"}"#).unwrap();
        first_json_value.merge(secound_json_value);
        assert_eq!(
            r#"{"value1":"a","value2":"c","value3":"d"}"#,
            first_json_value.to_string()
        );
    }

    #[test]
    fn it_should_merge_string() {
        let mut value_a: Value = Value::String("a".to_string());
        let value_b: Value = Value::String("b".to_string());
        value_a.merge(value_b.clone());
        assert_eq!(value_b.to_string(), value_a.to_string());
    }
}

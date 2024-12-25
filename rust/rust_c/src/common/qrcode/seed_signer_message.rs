use alloc::string::String;
use app_utils::impl_public_struct;

#[derive(Debug, Clone, Default)]
pub enum MessageEncoding {
    #[default]
    ASCII,
}

impl_public_struct!(SeedSignerMessage {
    path: String,
    message: String,
    encoding: MessageEncoding
});

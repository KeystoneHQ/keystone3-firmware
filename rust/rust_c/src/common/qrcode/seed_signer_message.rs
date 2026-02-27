use alloc::string::String;

#[derive(Debug, Clone, Default)]
pub enum MessageEncoding {
    #[default]
    ASCII,
}

#[derive(Debug, Clone)]
pub struct SeedSignerMessage {
    path: String,
    message: String,
    encoding: MessageEncoding,
}

impl SeedSignerMessage {
    pub fn new(path: String, message: String, encoding: MessageEncoding) -> Self {
        Self {
            path,
            message,
            encoding,
        }
    }

    pub fn get_path(&self) -> String {
        self.path.clone()
    }

    pub fn get_message(&self) -> String {
        self.message.clone()
    }

    pub fn get_encoding(&self) -> MessageEncoding {
        self.encoding.clone()
    }
}

#[macro_export]
macro_rules! impl_public_struct {
    ($name: ident { $($field: ident: $t: ty), *}) => {
        #[derive(Debug, Clone, Default)]
        pub struct $name {
            $(
              $field: $t,
            )*
        }

        impl $name {
            pub fn new($($field: $t), *) -> Self {
                Self {
                    $(
                        $field
                    ), *
                }
            }
        }

        app_utils::paste::item! {
            impl $name {
                $(
                    pub fn [<get_ $field>](&self) -> $t {
                        self.$field.clone()
                    }
                    pub fn [<set_ $field>](&mut self, $field: $t) {
                        self.$field = $field
                    }
                )*
            }
        }
    }
}

#[macro_export]
macro_rules! impl_internal_struct {
    ($name: ident { $($field: ident: $t: ty), *}) => {
        #[derive(Debug, Clone, Default)]
        pub(crate) struct $name {
            $(
              $field: $t,
            )*
        }

        impl $name {
            pub(crate) fn new($($field: $t), *) -> Self {
                Self {
                    $(
                        $field
                    ), *
                }
            }
        }

        app_utils::paste::item! {
            impl $name {
                $(
                    pub(crate) fn [<get_ $field>](&self) -> $t {
                        self.$field.clone()
                    }
                    pub(crate) fn [<set_ $field>](&mut self, $field: $t) {
                        self.$field = $field
                    }
                )*
            }
        }
    }
}

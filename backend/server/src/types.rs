use std::fmt;

use crate::errors::UnpackError;
use crate::wire;

/// The possible 'type classes' that we care about for actions & variables
pub enum TypeClass {
    Invalid,
    None,
    Enum(Vec<String>),
    Bool,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
}

pub struct Type {
    pub name: Option<String>,
    pub type_class: TypeClass,
}

pub const BOOL: Type = Type {
    name: None,
    type_class: TypeClass::Bool,
};
pub const UINT8: Type = Type {
    name: None,
    type_class: TypeClass::Uint8,
};
pub const UINT16: Type = Type {
    name: None,
    type_class: TypeClass::Uint16,
};
pub const UINT32: Type = Type {
    name: None,
    type_class: TypeClass::Uint32,
};
pub const UINT64: Type = Type {
    name: None,
    type_class: TypeClass::Uint64,
};
pub const INT8: Type = Type {
    name: None,
    type_class: TypeClass::Int8,
};
pub const INT16: Type = Type {
    name: None,
    type_class: TypeClass::Int16,
};
pub const INT32: Type = Type {
    name: None,
    type_class: TypeClass::Int32,
};
pub const INT64: Type = Type {
    name: None,
    type_class: TypeClass::Int64,
};
pub const FLOAT: Type = Type {
    name: None,
    type_class: TypeClass::Float,
};
pub const DOUBLE: Type = Type {
    name: None,
    type_class: TypeClass::Double,
};

pub fn enum_type<S: AsRef<str>>(name: S, labels: &[S]) -> Type {
    Type {
        name: Some(String::from(name.as_ref())),
        type_class: TypeClass::Enum(labels.iter().map(|s| String::from(s.as_ref())).collect()),
    }
}

impl fmt::Display for Type {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &self.type_class {
            TypeClass::Invalid => write!(f, "invalid")?,
            TypeClass::None => write!(f, "none")?,
            TypeClass::Enum(labels) => {
                write!(f, "enum")?;

                if let Some(name) = &self.name {
                    write!(f, "/{}", name)?;
                }

                if !labels.is_empty() {
                    write!(f, " [{}]", labels.join(", "))?;
                }
            }
            TypeClass::Bool => write!(f, "bool")?,
            TypeClass::Uint8 => write!(f, "u8")?,
            TypeClass::Uint16 => write!(f, "u16")?,
            TypeClass::Uint32 => write!(f, "u32")?,
            TypeClass::Uint64 => write!(f, "u64")?,
            TypeClass::Int8 => write!(f, "i8")?,
            TypeClass::Int16 => write!(f, "i16")?,
            TypeClass::Int32 => write!(f, "i32")?,
            TypeClass::Int64 => write!(f, "i64")?,
            TypeClass::Float => write!(f, "float")?,
            TypeClass::Double => write!(f, "double")?,
        }

        // If the type has a name and it's not an enum, just tag it on at the end
        if !self.is_enum() {
            if let Some(name) = &self.name {
                write!(f, " ({})", name)?;
            }
        }

        Ok(())
    }
}

impl Type {
    pub fn is_enum(&self) -> bool {
        matches!(&self.type_class, TypeClass::Enum(_))
    }

    pub fn pack(&self) -> wire::Type {
        let mut proto = wire::Type {
            r#type: 0,
            name: self.name.as_ref().unwrap_or(&String::from("")).clone(),
            labels: if let TypeClass::Enum(labels) = &self.type_class {
                labels.clone()
            } else {
                vec![]
            },
        };

        proto.set_type(match self.type_class {
            // Generated using vim macros :^)
            TypeClass::Invalid => wire::r#type::Class::Invalid,
            TypeClass::None => wire::r#type::Class::None,
            TypeClass::Enum(_) => wire::r#type::Class::Enum,
            TypeClass::Bool => wire::r#type::Class::Bool,
            TypeClass::Uint8 => wire::r#type::Class::Uint8,
            TypeClass::Uint16 => wire::r#type::Class::Uint16,
            TypeClass::Uint32 => wire::r#type::Class::Uint32,
            TypeClass::Uint64 => wire::r#type::Class::Uint64,
            TypeClass::Int8 => wire::r#type::Class::Int8,
            TypeClass::Int16 => wire::r#type::Class::Int16,
            TypeClass::Int32 => wire::r#type::Class::Int32,
            TypeClass::Int64 => wire::r#type::Class::Int64,
            TypeClass::Float => wire::r#type::Class::Float,
            TypeClass::Double => wire::r#type::Class::Double,
        });

        proto
    }

    pub fn unpack(proto: &wire::Type) -> Result<Self, UnpackError> {
        // Error if we have a non-enum type with labels, so that we don't have any weird bugs later
        // expecting this.
        if proto.r#type() != wire::r#type::Class::Enum && !proto.labels.is_empty() {
            return Err(UnpackError::LabelsError);
        }

        Ok(Type {
            name: if proto.name.is_empty() {
                None
            } else {
                Some(proto.name.clone())
            },

            type_class: match proto.r#type() {
                // Generated using vim macros :^)
                wire::r#type::Class::Invalid => TypeClass::Invalid,
                wire::r#type::Class::None => TypeClass::None,
                wire::r#type::Class::Enum => TypeClass::Enum(proto.labels.clone()),
                wire::r#type::Class::Bool => TypeClass::Bool,
                wire::r#type::Class::Uint8 => TypeClass::Uint8,
                wire::r#type::Class::Uint16 => TypeClass::Uint16,
                wire::r#type::Class::Uint32 => TypeClass::Uint32,
                wire::r#type::Class::Uint64 => TypeClass::Uint64,
                wire::r#type::Class::Int8 => TypeClass::Int8,
                wire::r#type::Class::Int16 => TypeClass::Int16,
                wire::r#type::Class::Int32 => TypeClass::Int32,
                wire::r#type::Class::Int64 => TypeClass::Int64,
                wire::r#type::Class::Float => TypeClass::Float,
                wire::r#type::Class::Double => TypeClass::Double,
            },
        })
    }
}

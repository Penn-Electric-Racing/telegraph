use std::fmt;

use crate::wire;
use crate::errors::UnpackError;

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
    pub name: String,
    pub type_class: TypeClass,
}

impl fmt::Display for Type {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match &self.type_class {
            TypeClass::Invalid => write!(f, "invalid")?,
            TypeClass::None => write!(f, "none")?,
            TypeClass::Enum(labels) => {
                write!(f, "enum")?;

                if self.name.len() > 0 {
                    write!(f, "/{}", self.name)?;
                }

                if labels.len() > 0 {
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
        if !self.is_enum() && self.name.len() > 0 {
            write!(f, " ({})", self.name)?;
        };

        Ok(())
    }
}

impl Type {
    pub fn is_enum(&self) -> bool {
        if let TypeClass::Enum(_) = &self.type_class {
            true
        } else {
            false
        }
    }

    pub fn pack(&self) -> wire::Type {
        let mut proto = wire::Type {
            r#type: 0,
            name: self.name.clone(),
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
        if proto.r#type() != wire::r#type::Class::Enum && proto.labels.len() > 0 {
            return Err(UnpackError::LabelsError)
        }

        Ok(Type {
            name: proto.name.clone(),

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


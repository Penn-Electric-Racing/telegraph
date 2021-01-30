use std::error::Error;
use std::fmt;

#[derive(Debug)]
pub enum UnpackError {
    NoneError,
    LabelsError,
}

impl fmt::Display for UnpackError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        // TODO: we might want more descriptive error messages
        match self {
            UnpackError::NoneError => write!(f, "tried to unpack an empty node!"),
            UnpackError::LabelsError => write!(f, "tried to unpack a non-enum type with labels!"),
        }
    }
}

impl Error for UnpackError {}

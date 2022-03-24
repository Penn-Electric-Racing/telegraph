use snafu::prelude::*;

#[derive(Debug, Snafu)]
pub enum ParseError {
    #[snafu(display("Fell off of the end of a stream while trying to read a value."))]
    FellOffEnd,

    #[snafu(display("Tried to parse an enum but got an unrecognized variant."))]
    UnrecognizedVariant,
}

pub trait FromBytes: Sized {
    fn read_from<B: Iterator<Item = u8>>(bytes: &mut B) -> Result<Self, ParseError>;
}

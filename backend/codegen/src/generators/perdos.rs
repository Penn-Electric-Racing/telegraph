use std::collections::{HashMap, HashSet};

use log::warn;

use per::assigned_perdos_ids::{AssignedPerDosIds, PerDosId};
use per::devices::{Device, Platform, UpdateType};
use per::value::Value;

use super::CodeGenerator;

const PERDOS_INCLUDES: &'static str = r#"#include "Uart.hpp""#;
const PERDOS_FUNCTIONS: &'static str = include_str!("perdos_functions.cpp");

pub struct PerDosCodeGenerator<'a> {
    perdos_ids: &'a AssignedPerDosIds,
}

fn perdos_send_id(device_perdos_ids: &Vec<&PerDosId>) -> String {
    let ids: HashMap<u32, &PerDosId> = device_perdos_ids
        .into_iter()
        .map(|id| (id.id, *id))
        .collect();

    let decls: String = (0x20..(ids.keys().max().unwrap() + 1))
        .map(|id| {
            if let Some(perdos_id) = ids.get(&id) {
                format!("&{}", perdos_id.value.value_name())
            } else {
                "nullptr".to_owned()
            }
        })
        .collect::<Vec<String>>()
        .join(",\n    ");

    format!(
        "constexpr const IValue* const perdosIds[] =
{decls}
"
    )
}

/// We multiply the estimated bitate by a factor to account for dropped packets and stuff.
/// TODO figure out why such a high scale factor is required. It shouldn't waste this much bus capacity.
const PERDOS_BITRATE_SCALE_FACTOR: f64 = 4.5;

fn perdos_send_lookup(
    device: &Device,
    device_perdos_ids: &Vec<&PerDosId>,
    number: usize,
) -> String {
    let bitrate = PERDOS_BITRATE_SCALE_FACTOR
        * device_perdos_ids
            .into_iter()
            .map(|id| -> f64 {
                id.value.per_dos_frequency * ((std::cmp::min(id.value.ty.size(), 8) + 16) as f64)
            })
            .sum::<f64>();

    let baud = device.per_dos_uart.configs()[number].baud;
    let period_factor = f64::max(1., bitrate / (baud as f64));

    if period_factor > 1. {
        warn!(
            "{device_name} PerDos{number} data is slowed down by a factor of {period_factor},\
        because {bitrate} bits/s are requested, but the baud rate is only {baud} bit/s",
            device_name = device.name
        );
    }

    let perdos_init = device_perdos_ids
        .into_iter()
        .filter(|id| id.value.per_dos_frequency > 0.)
        .map(|id| -> String {
            let mut perdos_frequency = id.value.per_dos_frequency;

            if !id.devices.contains(device) {
                // If this is not the device that is creating the variable,
                // there is no point in broadcasting it over the serial bus
                // faster than it is coming in over the CAN bus

                let can_frequency = id.value.can_frequency.max().unwrap_or(f64::INFINITY);
                if can_frequency < perdos_frequency {
                    perdos_frequency = can_frequency;
                }
            }

            let period = std::cmp::max(
                1,
                (period_factor * device.update_frequency / perdos_frequency) as u64,
            );

            format!(
                "PerDosSendInfo({id}, {period}), //{access_string}",
                id = id.id,
                access_string = id.value.access_string
            )
        })
        .collect::<Vec<_>>()
        .join("\n    ");

    format!(
        "PerDosSendInfo perdosSendInfo{number}[] =
{{
    {perdos_init}
}}
"
    )
}

impl<'a> CodeGenerator for PerDosCodeGenerator<'a> {
    fn is_used_by(&self, d: &Device) -> bool {
        !d.per_dos_uart.is_empty()
    }

    fn to_header(&self, d: &Device) -> String {
        let update_type = &d.update;
        match update_type {
            UpdateType::ManualUpdate => "".to_owned(),
            UpdateType::ManualFunctions => (0..d.per_dos_uart.num_devices() - 1)
                .map(|i| {
                    format!(
                        "void ReceivePerDos{i}();\nvoid TransmitPerDos{i}(bool alwaysTransmit);",
                    )
                })
                .collect::<Vec<_>>()
                .join("\n"),
        }
    }
    fn get_header_includes(&self, d: &Device) -> String {
        "".to_owned()
    }

    fn to_cpp(&self, d: &Device, values: &HashSet<Value>) -> String {
        let device_perdos_ids: Vec<&PerDosId> = self
            .perdos_ids
            .ids
            .iter()
            .filter(|id| values.contains(&id.value))
            .collect();

        let send_id = perdos_send_id(&device_perdos_ids);

        let send_lookups = d
            .per_dos_uart
            .configs()
            .into_iter()
            .enumerate()
            .map(|(i, _uart)| perdos_send_lookup(d, &device_perdos_ids, i))
            .collect::<Vec<_>>()
            .join("\n");

        let peripheral_initializers = d
            .per_dos_uart
            .configs()
            .into_iter()
            .enumerate()
            .map(|(i, uart)| {
                let rx_buffer_size =
                    std::cmp::max(100, (uart.baud as f64 / 8.0 / d.update_frequency) as u64);
                let tx_buffer_size = 100
                    + (1.1
                        * (&device_perdos_ids).into_iter().fold(0., |acc, id| {
                            let id_size = if id.id >= 0x100 { 2. } else { 1. };

                            acc + id_size + 1. + (id.value.ty.size() as f64 + 7.) / 8.
                        })) as u64;

                let uart_constructor = format!(
                    "peripheral::Uart perdos{i}Uart({rx}, {tx}, {rx_size}, {tx_size}, {baud});",
                    rx = uart.rx_pin,
                    tx = uart.tx_pin,
                    rx_size = rx_buffer_size,
                    tx_size = tx_buffer_size,
                    baud = uart.baud
                );

                let perdos_constructor = format!(
                    "PerDos perdos{i}(perdos{i}Uart, \
                        sizeof_array(perdosSendInfo{i}), \
                        perdosSendInfo{i});"
                );

                format!("{}\n{}", uart_constructor, perdos_constructor)
            })
            .collect::<Vec<_>>()
            .join("\n");

        format!(
            "{send_id}

{PERDOS_FUNCTIONS}

{send_lookups}

{peripheral_initializers}
"
        )
    }
    fn get_cpp_includes(&self, _d: &Device) -> String {
        PERDOS_INCLUDES.to_owned()
    }

    fn to_update(&self, d: &Device, indent: &str) -> String {
        d.per_dos_uart
            .configs()
            .into_iter()
            .enumerate()
            .map(|(n, uart)| match d.update {
                UpdateType::ManualFunctions => {
                    format!(
                        "\
void ReceivePerDos{n}(){{
    perdos{n}.Receive();
}}
void TransmitPerDos{n}(bool alwaysTransmit){{
    perdos{n}.Transmit(alwaysTransmit);
}}"
                    )
                }
                UpdateType::ManualUpdate => {
                    format!("{indent}perdos{n}.Receive();\n{indent}perdos{n}.Transmit(false);")
                }
            })
            .collect::<Vec<_>>()
            .join("\n")
    }
}

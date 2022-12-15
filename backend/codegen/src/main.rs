mod generators;

use std::collections::HashMap;
use std::path::PathBuf;

use argh::FromArgs;
use eyre::eyre;
use log::{info, trace, warn};

use per::{
    assigned_can_ids::AssignedCanIds,
    assigned_perdos_ids::AssignedPerDosIds,
    devices::{Device, Devices, Platform},
};

use generators::*;

#[derive(FromArgs)]
/// PER's custom code generator.
struct Args {
    #[argh(switch, short = 'v')]
    /// displays debug information
    verbose: bool,
    #[argh(switch, short = 'r')]
    /// recreates all of the generated files, even if they are up to date
    rebuild: bool,
    #[argh(switch, short = 'c')]
    /// removes all of the generated files
    clean: bool,
    #[argh(option)]
    /// specifies the configuration file to use
    config: PathBuf,
    #[argh(option)]
    /// specifies the working directory
    directory: Option<PathBuf>,
    #[argh(option)]
    /// specifies a file with the existing perdos ids
    existing_perdos: Option<PathBuf>,
    #[argh(option)]
    /// specifies a file with the existing can ids
    existing_can: Option<PathBuf>,

    #[argh(switch)]
    /// pack multiple values per CAN message. Note that this may create issues when an id is later removed
    optimize_can: bool,

    #[argh(switch)]
    /// old CAN ids can be removed and reused. Note that every board needs to be re-flashed when using this
    remove_old_can_ids: bool,
}

fn main() -> eyre::Result<()> {
    let args: Args = argh::from_env();

    let env = env_logger::Env::default().filter_or(
        "CODEGEN_LOG_LEVEL",
        if args.verbose { "debug" } else { "info" },
    );
    env_logger::init_from_env(env);

    let mut should_rebuild = args.rebuild;

    if let Some(directory) = args.directory {
        trace!(
            "Changing the working directory to {} from the command-line",
            directory.to_string_lossy()
        );
        std::env::set_current_dir(directory)?;
    }

    let root_directory = std::env::current_dir()?;

    let input_files: Vec<PathBuf> = [
        Some(&args.config),
        args.existing_can.as_ref(),
        args.existing_perdos.as_ref(),
    ]
    .iter()
    .filter_map(|f| *f)
    .map(|f| f.clone())
    .collect();

    let configuration = Devices::read_from(std::fs::File::open(&args.config)?)?;

    let internal_devices = configuration
        .devices
        .iter()
        .filter(|d| d.platform != Platform::External);

    let generated_folders: HashMap<&Device, PathBuf> = internal_devices
        .map(|d| {
            let mut path = root_directory.clone();
            path.push(d.name.replace(" ", "-"));
            path.push("Generated");
            (d, path)
        })
        .collect();

    if args.clean {
        for folder in generated_folders.values() {
            std::fs::remove_dir_all(folder)?;
        }

        info!("Successfully removed all generated files!");
        std::process::exit(0);
    }

    let last_generated = generated_folders
        .values()
        .filter(|d| d.exists())
        .filter_map(|d| std::fs::read_dir(d).ok())
        .flatten()
        .filter_map(|e| e.ok()?.metadata().ok()?.modified().ok())
        .min()
        .unwrap_or(std::time::SystemTime::UNIX_EPOCH);

    let executable_path = std::env::args()
        .nth(0)
        .ok_or(eyre!("Missing 0th argument, how did we get here?"))?;

    if last_generated < std::fs::metadata(executable_path)?.modified()? {
        should_rebuild = true;
    } else {
        for file in input_files {
            if last_generated < std::fs::metadata(file)?.modified()? {
                should_rebuild = true;
                break;
            }
        }
    }

    if !should_rebuild {
        info!("Generated code is already up to date!");
        std::process::exit(0);
    }

    let existing_per_dos_ids = if let Some(perdos_file) = args.existing_perdos {
        let perdos_file = std::fs::File::open(perdos_file)?;
        Some(AssignedPerDosIds::read_from(perdos_file)?)
    } else {
        None
    };

    let existing_can_ids = if let Some(can_file) = args.existing_can {
        let can_file = std::fs::File::open(can_file)?;
        Some(AssignedCanIds::read_from(can_file)?)
    } else {
        None
    };

    // Dictionary<Device, HashSet<Value>> deviceValues = configuration.Devices.ToDictionary(
    //     device => device,
    //     device => new HashSet<Value>(
    //         configuration.StoredValues(device, prefixes[device.Name])
    //             .Where(value =>
    //                 device.InstanceIdentifiers().Any(value.DeviceIdentifier().Equals)
    //                 || (device.Settings.Get<CanConfigurationSetting>()?.Cans.Any(c =>
    //                 {
    //                     Device canDevice = configuration.Devices.Single(d =>
    //                         d.InstanceIdentifiers().Any(value.DeviceIdentifier().Equals));

    //                     if (!CanCodeGenerator.UsesGeneratorStatic(canDevice))
    //                         return false;

    //                     if (!canDevice.Settings.Get<CanConfigurationSetting>().Cans.Select(can => can.BusName).Contains(c.BusName))
    //                         return false;

    //                     var frequencySetting = value.Settings.Get<CanFrequencySetting>();
    //                     return frequencySetting != null
    //                         &&    (frequencySetting.Frequencies.TryGetValue(c.BusName, out double freq)
    //                             || frequencySetting.Frequencies.TryGetValue(CanConfig.EveryBus, out freq))
    //                         && freq > 0;
    //                     }) ?? false)
    //                 )));

    // CanIdLogFile assignedCanIds = CanIdLogFile.AssignIds(existingCanIds, configuration,
    //     optimizeCan, removeOldCanIds, out List<Tuple<CanId, string>> idErrors);

    // if (idErrors.Count > 0)
    // {
    //     Console.WriteLine("Error assigning CAN ids!");
    //     foreach (var error in idErrors)
    //     {
    //         Console.WriteLine((error.Item1 != null ? $"ID {error.Item1.Id}: " : "") + error.Item2);
    //     }

    //     return 1;
    // }

    // if (assignedCanIds.CanIds.Any() && !string.IsNullOrWhiteSpace(existingCanIdFile))
    //     assignedCanIds.ToFile(existingCanIdFile);

    // TODO
    warn!("Assigning CAN ids is not yet implemented... ignoring new CAN ids");

    // PerDosIdFile assignedPerDosIds =
    //     PerDosIdFile.AssignIds(existingPerDosIds, configuration, deviceValues);

    // if (assignedPerDosIds.Ids.Any() && !string.IsNullOrWhiteSpace(existingPerdosIdFile))
    //     assignedPerDosIds.ToFile(existingPerdosIdFile);

    // TODO
    warn!("Assigning PerDOS ids is not yet implemented... ignoring new PerDOS ids");

    // CanCodeGenerator.AddItems(configuration);
    // List<CodeGenerator> allCodeGenerators = new List<CodeGenerator>
    // {
    //     new CanCodeGenerator(assignedCanIds, configuration.Devices),
    //     new PerDosCodeGenerator(assignedPerDosIds),
    // };

    // Dictionary<Device, List<CodeGenerator>> deviceCodeGenerators =
    //     notExternalDevices.ToDictionary(d => d,
    //         d => allCodeGenerators.Where(c => c.UsesGenerator(d)).ToList());

    // ParallelOptions options = new ParallelOptions
    // {
    //     MaxDegreeOfParallelism = Debugger.IsAttached ? 1 : Environment.ProcessorCount
    // };
    // Parallel.ForEach(notExternalDevices, options,
    //     device =>
    //     {
    //         string directory = generatedFolders[device];
    //         if (Directory.Exists(directory))
    //         {
    //             foreach (string file in Directory.EnumerateFiles(directory))
    //             {
    //                 for (int i = 0; i < 10; i++)
    //                 {
    //                     //Ocassionally files can't be deleted, probably because a program (visual studio?) is using them
    //                     try
    //                     {
    //                         File.Delete(file);
    //                         break;
    //                     }
    //                     catch (Exception)
    //                     {
    //                         Thread.Sleep(25);

    //                         if (i == 9)
    //                             throw;
    //                     }
    //                 }
    //             }
    //         }

    //         Directory.CreateDirectory(directory);

    //         List<CodeGenerator> usedCodeGenerators = deviceCodeGenerators[device];

    //         HashSet<Value> currentDeviceValues = deviceValues[device];

    //         string headerString = configuration.ToHeader(device, currentDeviceValues, usedCodeGenerators);
    //         string cppString = configuration.ToCpp(device, currentDeviceValues, usedCodeGenerators);

    //         using (var writer = new StreamWriter(
    //             File.Open(Path.Combine(directory, device.UppercaseIdentifier() + "Registers.hpp"),
    //                 FileMode.Create)))
    //         {
    //             writer.Write(headerString);
    //         }

    //         using (var writer = new StreamWriter(
    //             File.Open(Path.Combine(directory, device.UppercaseIdentifier() + "Registers.cpp"),
    //                 FileMode.Create)))
    //         {
    //             writer.Write(cppString);
    //         }
    //     });

    // TODO
    warn!("Generating code is not yet implemented... ignoring new PerDOS ids");

    info!("Code generation completed successfully!");

    Ok(())
}

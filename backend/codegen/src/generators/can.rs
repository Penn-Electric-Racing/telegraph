use std::collections::HashSet;

use per::assigned_perdos_ids::AssignedPerDosIds;
use per::devices::{Device, UpdateType};
use per::value::Value;

use super::CodeGenerator;

pub struct CanCodeGenerator {}

impl CodeGenerator for CanCodeGenerator {
    fn is_used_by(&self, d: &Device) -> bool {
        d.can_configuration.is_some()
    }

    fn to_header(&self, d: &Device) -> String {
        todo!()
    }
    fn get_header_includes(&self, d: &Device) -> String {
        todo!()
    }

    fn to_cpp(&self, d: &Device, values: &HashSet<Value>) -> String {
        todo!()
    }
    fn get_cpp_includes(&self, d: &Device) -> String {
        todo!()
    }

    fn to_update(&self, d: &Device, indent: &str) -> String {
        d.can_configuration
            .configs()
            .into_iter()
            .enumerate()
            .map(|(n, uart)| match d.update {
                UpdateType::ManualFunctions => {
                    format!(
                        "\
void ReceivePerDos{n}(){{
{indent}perdos{n}.Receive();
}}
void TransmitPerDos{n}(bool alwaysTransmit){{
{indent}perdos{n}.Transmit(alwaysTransmit);
}}"
                    );
                }
                UpdateType::ManualUpdate => {
                    format!("{indent}perdos{n}.Receive();\n{indent}perdos{n}.Transmit(false);")
                }
            })
            .collect::<Vec<_>>()
            .join("\n")

        // return indent + string.Join("\r\n" + indent,
        //             updateType != UpdateSetting.UpdateType.ManualFunctions
        //                 ? device.Settings.Get<CanConfigurationSetting>().Cans
        //                     .Select(c =>
        //                         string.Concat(device.Ids.Select(id =>
        //                                 $"if(DeviceId == {id}){{\r\n" +
        //                                 $"{indent}    perdoc{id}{c.BusName}.Receive();\r\n" +
        //                                 $"{indent}    perdoc{id}{c.BusName}.Transmit();\r\n" +
        //                                 $"{indent}}}\r\n" +
        //                                 $"{indent}else "))
        //                         + "Breakpoint();")
        //                     .Concat(UpdateTimeouts(device))
        //                 : device.Settings.Get<CanConfigurationSetting>().Cans
        //                     .Select(c =>
        //                         $"\r\nvoid ReceiveCan{c.BusName}(){{\r\n    " +
        //                         string.Concat(device.Ids.Select(id =>
        //                             $"{indent}if(DeviceId == {id}){{\r\n" +
        //                             $"{indent}        perdoc{id}{c.BusName}.Receive();\r\n" +
        //                             $"{indent}    }}\r\n" +
        //                             $"{indent}    else "
        //                         )) +
        //                         "Breakpoint();\r\n\r\n" +
        //                         string.Concat(UpdateTimeouts(device).Select(t =>$"{indent}    {t}\r\n")) +
        //                         "}\r\n" +
        //                         $"void TransmitCan{c.BusName}(){{\r\n    " +
        //                         string.Concat(device.Ids.Select(id =>
        //                             $"{indent}if(DeviceId == {id}){{\r\n" +
        //                             $"{indent}        perdoc{id}{c.BusName}.Transmit();\r\n" +
        //                             $"{indent}    }}\r\n" +
        //                             $"{indent}    else "
        //                         )) +
        //                         "Breakpoint();\r\n" +
        //                         "}"));

        // todo!()
    }
}

// namespace PER.Files.Can
// {
//     public class CanCodeGenerator : CodeGenerator
//     {
//         public CanIdLogFile CanIds { get; }
//         private List<Device> Devices { get; }

//         public CanCodeGenerator(CanIdLogFile canIds, List<Device> devices)
//         {
//             CanIds = canIds;
//             Devices = devices.Where(UsesGeneratorStatic).ToList();
//         }

//         public static void AddItems(ConfigurationFile configuration)
//         {
//             List<Device> devices = configuration.Devices
//                 .Where(UsesGeneratorStatic)
//                 .ToList();

//             foreach (Device device in devices.Where(d => d.Platform != Platform.External))
//             {
//                 var connectedBuses = device.Settings.Get<CanConfigurationSetting>().Cans.Select(c => c.BusName).ToList();
//                 double canFrequency = CanFrequency(device);
//                 device.AddItems(
//                     devices.Except(device.Ids.Count > 1 ? Enumerable.Empty<Device>() : new[] {device})
//                         .Where(d => d.Settings.Get<CanConfigurationSetting>().Cans
//                                      .Select(c => c.BusName)
//                                      .Intersect(connectedBuses)
//                                      .Any())
//                         .SelectMany(d => d.Names
//                             .Where((name, i) => !device.Values().Any(v =>
//                                 v.DirectAccessString.EndsWith(d.InstanceIdentifiers().ElementAt(i) + "CanConnected")))
//                             .Select((name, i) => new Variable<bool>(
//                                 name + " Connected over CAN", d.InstanceIdentifiers().ElementAt(i) + "CanConnected",
//                                 $"True if the {name} is connected", new SettingGroup<CodeGenerationSetting>(
//                                     new[] {new CanFrequencySetting(
//                                         new Dictionary<string, double>{{ CanConfig.EveryBus, canFrequency } })}))))
//                         .Cast<Item>()
//                         .ToList()
//                 );
//             }
//         }

//         private static double CanFrequency(Device device)
//         {
//             return device.Platform == Platform.LPC1768 || device.Platform == Platform.External
//                  ? device.Settings.Get<CanBaseFrequencySetting>().BaseFrequency
//                  : device.Settings.Get<UpdateFrequencySetting>().Frequency;
//         }
//         private string ToLookupTableLpc1768(Device device, ICollection<Value> storedValues)
//         {
//             var relevantIds = CanIds.Where(canId => canId.ContainsAny(storedValues)).ToList();
//             string result = string.Concat(relevantIds.Select(canId => canId.ToCppAccessArray(storedValues)));
//             uint previousId = 0x100 - 1;

//             result += "\r\nconst CanId canIds[] = {\r\n";
//             foreach (CanId canId in relevantIds)
//             {
//                 result += string.Concat(Enumerable.Repeat("EID,\r\n", (int)(canId.Id - previousId - 1)));
//                 previousId = canId.Id;
//                 result += $"{{{canId.Values.Count}, {canId.DeviceId}, canId{canId.Id}}},\r\n";
//             }
//             result += "};\r\n\r\n";

//             int controller = device.Settings.Get<CanControllerSetting>().ControllerNumber;
//             PinBaud configuration = device.Settings.Get<CanConfigurationSetting>().Cans[0];
//             string speed = configuration.Baud == 1000000 ? "HIGH" : configuration.Baud == 250000 ? "LOW" : "MEDIUM";
//             string pinsController = configuration.Rx == "_" && configuration.Tx == "_"
//                                   ? "CAN" + controller
//                                   : configuration.Rx + ", " + configuration.Tx;
//             string silent = device.Settings.Get<CanSilentSetting>()?.Pin ?? "NC";

//             result += $"CANBuffer canBuffer({pinsController}, {speed}, {silent}, {5 + CanIds.Select(i => i.DeviceId).Count(device.Ids.Contains)}, " +
//                       $"{5 + (int) Math.Ceiling(configuration.Baud / 76.0 / CanFrequency(device))});\r\n\r\n";
//             return result;
//         }

//         private string ToLookupTableStm32(Device device, ICollection<Value> storedValues)
//         {
//             var relevantIds = CanIds.Where(canId => canId.ContainsAny(storedValues)).ToList();
//              Dictionary<uint, CanId> ids = relevantIds.ToDictionary(id => id.Id);
//              return string.Join("\r\n",
//                         relevantIds.Where(id => id.Values.Count > 1)
//                             .Select(c => $"constexpr ValueGroup<{c.Values.Count}> canId_{c.Id}_group({BitFieldLayout.FromValues(c.Values).MergedLength}, {{ " +
//                                          string.Join(", ", c.Values.Select(v =>
//                                              storedValues.Contains(v)
//                                                  ? "&" + v.ValueName
//                                                  : $"unknownValue<{VariableTypes.CppTypeNameDictionary[v.Type]}>")) +
//                                          " });")) +
//                     "\r\n\r\n" +
//                     "constexpr CanId canIds[] =\r\n" +
//                     "{\r\n    " +
//                     string.Join(",\r\n    ",
//                         Enumerable.Range(0, (int)(ids.Keys.Any() ? ids.Keys.Max() + 1 : 0))
//                             .Select(id =>
//                                 !ids.TryGetValue((uint)id, out CanId canId)
//                                     ? "{0, nullptr}"
//                                     : $"{{{canId.DeviceId}, " + (
//                                       canId.Values.Count == 1
//                                         ? $"&{canId.Values[0].ValueName}}}"
//                                         : $"&canId_{id}_group}}"))
//                     ) +
//                     "\r\n};\r\n\r\n";
//         }

//         private IEnumerable<string> UpdateTimeouts(Device device)
//         {
//             return device.InstanceValues()
//                 .Where(v => v.DirectAccessString.EndsWith("CanConnected"))
//                 .Select(v =>
//                 {
//                     uint otherId = Devices.SelectMany(d =>
//                             d.InstanceIdentifiers().Select((id, i) => (DeviceId: d.Ids.ElementAt(i), identifier: id)))
//                         .Single(t => v.DirectAccessString.EndsWith(t.identifier + "CanConnected")).DeviceId;
//                     int periodIndex = v.DirectAccessString.IndexOf('.', 0);
//                     string accessString = device.Ids.Count == 1
//                         ? v.DirectAccessString
//                         : v.DirectAccessString.Remove(periodIndex, 1).Insert(periodIndex, "->");
//                     return $"{accessString} = deviceConnected({otherId}, {accessString});";
//                 });
//         }
//         private string TimeoutTables(Device thisDevice)
//         {
//             Dictionary<uint, List<CanId>> deviceCanIds = CanIds.GroupBy(canId => canId.DeviceId)
//                 .Where(g => Devices.Any(d => d.Ids.Contains(g.Key)))
//                 .ToDictionary(g=> g.Key, g => g.ToList());

//             Dictionary<uint, double> timeoutDelays =
//                 deviceCanIds.ToDictionary(kvp => kvp.Key,
//                     kvp => 1 / kvp.Value.Select(canId => ReceivedFrequencies(thisDevice, canId.SendFrequencies).Max())
//                                         .Where(f => f > 0)
//                                         .DefaultIfEmpty(0)
//                                         .Min());

//             int maxId = (int) (Devices.SelectMany(d => d.Ids).Max() + 1);
//             return "static uint32_t canTimeoutDelays[] = {\r\n" +
//                    string.Join("\r\n", Enumerable.Range(0, maxId)
//                        .Select(id => (uint) id)
//                        .Select(id => !timeoutDelays.TryGetValue(id, out double timeoutDelay) || double.IsInfinity(timeoutDelay)
//                            ?  "    0,"
//                            :  $"    1 + {(thisDevice.Platform == Platform.STM32 ? "static_cast<uint32_t>" : "(uint32_t)")}" +
//                               $"(1.25f*configTICK_RATE_HZ*static_cast<float>({timeoutDelay})),")) +
//                    "\r\n};\r\n" +
//                    "static uint32_t canTimeouts[] = {\r\n    " +
//                    string.Join(", ", Enumerable.Repeat("0", maxId)) +
//                    "\r\n};\r\n\r\n" +
//                    "static constexpr uint32_t expectedMessageCounts[] = {\r\n" +
//                     string.Join("\r\n", Enumerable.Range(0, maxId)
//                         .Select(id => (uint) id)
//                         .Select(id =>
//                         {
//                             uint value = !timeoutDelays.TryGetValue(id, out double timeoutDelay) || double.IsInfinity(timeoutDelay)
//                                 ? uint.MaxValue
//                                 : (uint) deviceCanIds[id]
//                                     .Sum(canId => ReceivedFrequencies(thisDevice, canId.SendFrequencies).Sum() * timeoutDelays[id]);
//                             return $"    {value},";
//                         })) +
//                     "\r\n};\r\n\r\n" +
//                     "static uint32_t canConnectionMessageCounters[] = {\r\n    " +
//                     string.Join(", ", Enumerable.Repeat("0", maxId)) +
//                     "\r\n};\r\n\r\n;"
//                     + CppFunctions.CanDeviceConnected;
//         }

//         IEnumerable<double> ReceivedFrequencies(Device device, IReadOnlyDictionary<string, double> frequencies)
//         {
//             return device.Settings.Get<CanConfigurationSetting>().Cans
//                          .Select(c => frequencies.TryGetValue(c.BusName, out double f) ? f : double.NaN)
//                          .Where(f => !double.IsNaN(f))
//                          .DefaultIfEmpty(0);
//         }

//         private string ToUpdateFunctions(Device device, string indent)
//         {
//             CanId firstMutlipleId = CanIds.FirstOrDefault(canId => device.Ids.Contains(canId.DeviceId) && canId.SendFrequencies.Count != 1);
//             if (firstMutlipleId != null)
//             {
//                 throw new ArgumentException("LPC1768s don't support multiple CAN frequencies for the same variable on different buses");
//             }
//             double baseFrequency = CanFrequency(device);
//             IOrderedEnumerable<IGrouping<int, CanId>> canIds =
//                 CanIds.Where(canId => device.Ids.Contains(canId.DeviceId))
//                       .GroupBy(canId => (int) Math.Ceiling(baseFrequency/canId.SendFrequencies.Values.First()))
//                       .OrderBy(g => g.Key);

//             string function = "void sendNextCanMessages(){\r\n" +
//                               $"{indent}{string.Join("\r\n    ", UpdateTimeouts(device))}\r\n\r\n" +
//                               $"{indent}static uint32_t sendIndex = 0;\r\n" +
//                               $"{indent}sendIndex++;\r\n";
//             function +=
//                 string.Concat(
//                     canIds.Select(idGroup =>
//                         $"{indent}switch(sendIndex%{idGroup.Key}){{\r\n" +
//                         string.Concat(
//                             idGroup.Select((id, i) => new { canId = id, index = i })
//                                 .GroupBy(b => b.index % idGroup.Key)
//                                 .Select(indexGroup =>
//                                     $"{indent}{indent}case {indexGroup.Key}:\r\n" +
//                                     string.Concat(
//                                         indexGroup.Select(
//                                             t => $"{indent}{indent}{indent}sendCanMessage({t.canId.Id});\r\n")
//                                         ) +
//                                     $"{indent}{indent}{indent}break;\r\n")
//                             ) +
//                         indent + "}\r\n"));

//             function += "}\r\n";
//             return function;
//         }

//         public override string HeaderIncludes(Platform platform)
//         {
//             return platform == Platform.LPC1768 ? CppFunctions.CanIncludes : "#include <Can.hpp>";
//         }
//         public override string CppIncludes(Platform platform)
//         {
//             return "";
//         }

//         public override string ToHeader(Device device)
//         {
//             UpdateSetting.UpdateType updateType =
//                 device.Settings.Get<UpdateSetting>()?.Update ?? UpdateSetting.UpdateType.AutoUpdate;

//             IEnumerable<string> canHandlers = device.Settings.Get<CanHandlersSetting>()?.CanHandlers ?? false
//                 ? device.Settings.Get<CanConfigurationSetting>().Cans
//                     .Select(c => c.BusName)
//                     .Select(b => $"void ReceivedCan{b}(CanMessage& message);\r\nbool TransmittedCan{b}(CanMessage& message);")
//                 : Enumerable.Empty<string>();

//             return device.Platform == Platform.LPC1768
//                  ? CppFunctions.CanFunctionPrototypes
//                  : updateType != UpdateSetting.UpdateType.ManualFunctions
//                  ? string.Join("\r\n", canHandlers)
//                  : string.Join("\r\n", device.Settings.Get<CanConfigurationSetting>().Cans
//                         .Select(c => c.BusName)
//                         .Select(b => $"void ReceiveCan{b}();\r\nvoid TransmitCan{b}();")
//                         .Concat(canHandlers));
//         }

//         public override Dictionary<string, string> ToOtherFiles(Device device, HashSet<Value> storedValues)
//         {
//             if(device.Platform == Platform.LPC1768)
//             {
//                 return new Dictionary<string, string>
//                 {
//                     {"CAN_Filter_LUT.h", ToCanFilter(device, storedValues) }
//                 };
//             }
//             return new Dictionary<string, string>();
//         }

//         private string ToCanFilter(Device device, HashSet<Value> storedValues)
//         {
//             List<uint> relevantIds = CanIds.Where(canId => canId.ContainsAny(storedValues))
//                                            .Where(canId => !device.Ids.Contains(canId.DeviceId))
//                                            .Select(canId => canId.Id).ToList();
//             relevantIds.Sort();

//             CanControllerSetting controller = device.Settings.Get<CanControllerSetting>();
//             if (relevantIds.Count == 0 || controller == null)
//             {
//                 return CppFunctions.CanFilters();
//             }

//             Range<uint> range = new Range<uint>(relevantIds[0], relevantIds[0]);
//             List<Range<uint>> completeRanges = new List<Range<uint>>();
//             foreach (uint id in relevantIds.Skip(1))
//             {
//                 if (id == range.Max + 1)
//                 {
//                     range = new Range<uint>(range.Min, range.Max + 1);
//                     continue;
//                 }
//                 completeRanges.Add(range);
//                 range = new Range<uint>(id, id);
//             }
//             completeRanges.Add(range);

//             List<Range<uint>> ranges = new List<Range<uint>>();
//             List<uint> individuals = new List<uint>();
//             foreach (Range<uint> r in completeRanges)
//             {
//                 if (r.Min == r.Max)
//                 {
//                     individuals.Add(r.Min);
//                 }
//                 else
//                 {
//                     ranges.Add(r);
//                 }
//             }

//             switch (controller.ControllerNumber)
//             {
//                 case 1:
//                     return CppFunctions.CanFilters(standardC1: individuals, standardRangeC1: ranges);
//                 case 2:
//                     return CppFunctions.CanFilters(standardC2: individuals, standardRangeC2: ranges);
//                 default:
//                     throw new ArgumentException($"Invalid {nameof(controller)}");
//             }
//         }

//         public override string ToCpp(Device device, HashSet<Value> storedValues)
//         {
//             if (device.Platform == Platform.STM32)
//             {
//                 return CppFunctions.CanStructs +
//                        ToLookupTableStm32(device, storedValues) +
//                        TimeoutTables(device) +
//                        CppFunctions.CanFunctions +
//                        SendInfoTable(device);
//             }

//             if (device.Platform == Platform.LPC1768)
//             {
//                 return CppFunctions.CanDefines +
//                        CppFunctions.OldCanStructs +
//                        ToLookupTableLpc1768(device, storedValues) +
//                        TimeoutTables(device) +
//                        CppFunctions.CanGettersSetters +
//                        CppFunctions.ReceiveCanMessageFunctions +
//                        CppFunctions.SendCanMessageFunctions +
//                        ToUpdateFunctions(device, CppFunctions.Indent()) +
//                        "\r\n";
//             }

//             return "";
//         }

//         private string SendInfoTable(Device device)
//         {
//             Dictionary<uint,Dictionary<string, List<CanId>>> deviceBusIds =
//                 device.Ids.ToDictionary(deviceId => deviceId, deviceId =>
//                     device.Settings.Get<CanConfigurationSetting>().Cans
//                         .Select(c => c.BusName)
//                         .ToDictionary(name => name, name =>
//                             CanIds.Where(id => deviceId == id.DeviceId
//                                            && id.SendFrequencies.TryGetValue(name, out double freq)
//                                            && freq > 0)
//                                   .ToList()));

//             double updateFrequency = CanFrequency(device);
//             return string.Join("\r\n", device.Settings.Get<CanConfigurationSetting>().Cans.SelectMany(c =>
//                        device.Ids.Where(deviceId => deviceBusIds.TryGetValue(deviceId, out var dict)
//                                                  && dict.TryGetValue(c.BusName, out var list)
//                                                  && list.Count > 0)
//                                  .Select(deviceId =>
//                     $"CanSendInfo canSendInfos{deviceId}{c.BusName}[] =\r\n{{" +
//                     string.Concat(
//                         deviceBusIds[deviceId][c.BusName]
//                             .GroupBy(id => (int)Math.Ceiling(updateFrequency / id.SendFrequencies[c.BusName]))
//                             .SelectMany(
//                         group =>
//                         {
//                             bool stagger = device.Settings.Get<CanStaggerSetting>()?.Stagger ?? false;
//                             int sendPeriod = group.Key;
//                             List<CanId> ids = group.ToList();
//                             double staggerOffset = stagger ? sendPeriod/(double)ids.Count : 0;
//                             return ids.Select((id, i) =>
//                                 $"\r\n    CanSendInfo({id.Id}, {sendPeriod}, {(((int)(i * staggerOffset)) % sendPeriod) + 1}), // " +
//                                 $"{string.Join(", ", id.Values.Select(v => v.DirectAccessString))}");
//                         })) +
//                     "\r\n};"
//                 ))) +
//                 "\r\n\r\n" +
//                 string.Join("\r\n", device.Settings.Get<CanConfigurationSetting>().Cans.Select(c =>
//                     $"peripheral::Can can{c.BusName}({c.Rx}, {c.Tx}, " +
//                     $"{10 + 2.5 * (int)Math.Ceiling(c.Baud / 60.0 / CanFrequency(device))}, " +
//                     $"{10 + 1.5 * device.Ids.Select(deviceId => CanIds.Count(id => deviceId == id.DeviceId && id.SendFrequencies.ContainsKey(c.BusName))).Max()}, " +
//                     $"{c.Baud});\r\n" +
//                     string.Join("\r\n", device.Ids.Select(id =>
//                         $"PerDoc perdoc{id}{c.BusName}(can{c.BusName}, {deviceBusIds[id][c.BusName].Count}, " +
//                         (deviceBusIds[id][c.BusName].Any() ? $"canSendInfos{id}{c.BusName}" : "nullptr") +
//                         ((device.Settings.Get<CanHandlersSetting>()?.CanHandlers ?? false)
//                             ? $", ReceivedCan{c.BusName}, TransmittedCan{c.BusName}"
//                             : ", nullptr, nullptr" )
//                         + ");"
//                     ))
//                 )) +
//                 "\r\n";
//         }

// }

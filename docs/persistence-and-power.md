# Persistence And Power Model

## Retained Configuration

`RetainedConfig` stores deployment-critical values:

- device instance,
- vendor ID,
- object base instance,
- setpoint bounds,
- occupied and unoccupied setpoints,
- transmit interval,
- BACnet MAC,
- BLE address,
- zone label,
- room label,
- commissioning state,
- checksum.

The host implementation writes a text file through `FileConfigStorage`. This intentionally behaves like a repository adapter so the same domain object can later use Nordic FDS or external EEPROM.

## Checksum

The checksum is a deterministic FNV-1a-style hash over the serialized config payload, excluding the checksum field itself.

Load validation rejects:

- bad magic,
- unsupported version,
- missing device/object identity,
- invalid setpoint bounds,
- empty labels,
- transmit intervals below 10 seconds,
- checksum mismatch.

## Battery Model

`BatteryEstimator` computes:

- battery percent from millivolts,
- low battery alarm below `2300 mV`,
- approximate runtime days from active current, sleep current, capacity, and sample interval.

Default profile:

| Field | Value |
| --- | ---: |
| Capacity | `2200 mAh` |
| Active current | `8.5 mA` |
| Sleep current | `7.0 uA` |
| Sample interval | `60 s` |

This is a portfolio-safe estimate, not a replacement for power-profiler measurements. Hardware validation should add measured current captures.

## Future Low-Power Firmware Work

- Replace host samples with SAADC battery readings.
- Gate BACnet/BLE transmit windows with the sample interval.
- Persist setpoint changes only when dirty to reduce flash wear.
- Add brownout-safe double-buffered config records.
- Add power-profiler captures for sleep, sample, BLE provisioning, and BACnet bridge publish modes.

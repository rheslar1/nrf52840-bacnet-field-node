# Validation Plan

## Current Host Checks

Run:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/nrf52840_bacnet_field_node --status
./build/nrf52840_bacnet_field_node --objects
```

The C++ tests validate:

- default config validity,
- BACnet object count,
- occupied setpoint clamp policy,
- provisioning,
- commissioning lock behavior,
- low-battery alarm object,
- retained config save/load round trip.

## CLI Evidence

Capture these commands:

```bash
./build/nrf52840_bacnet_field_node --commission 700001 Tower-B Server-Room
./build/nrf52840_bacnet_field_node --sample 24.2 52 2210 0
./build/nrf52840_bacnet_field_node --setpoint 90
./build/nrf52840_bacnet_field_node --objects
```

## Static Review Checklist

- C++17 only.
- No generated binary artifacts committed.
- Domain logic separated from CLI.
- Storage access goes through `IConfigStorage`.
- Setpoint behavior goes through `ISetpointPolicy`.
- BACnet mapping goes through `BacnetObjectMapper`.
- Commissioning rules are covered by tests.

## Hardware Validation To Add

- nRF52840 board target build.
- BLE commissioning characteristic test.
- Nordic FDS retained config test.
- SAADC battery sample test.
- I2C/SPI room sensor sample test.
- BACnet gateway integration test.
- Power-profiler sleep/sample/transmit captures.
- Brownout/reset retained-config recovery test.

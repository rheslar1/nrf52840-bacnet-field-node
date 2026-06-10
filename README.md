# nRF52840 BACnet Field Node

Battery-aware field device profile with persistent setpoint storage, commissioning evidence, and BACnet object mapping.

## Portfolio Purpose

This repository is an Embedded Systems project scaffold for the Rheslar portfolio. It is designed to become a hardware-backed project with build output, validation logs, and reviewable implementation evidence.

## Stack

- nRF52840
- C
- EEPROM
- BACnet
- BLE-ready

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/nrf52840_bacnet_field_node
python -m unittest discover -s tests
```

## Implementation Slices

- Native starter executable that exposes the project identity, stack, and validation target.
- Architecture document with control boundaries, data flow, safety assumptions, and evidence plan.
- Unit smoke test that keeps source, docs, and CI files present as the repo grows.
- GitHub Actions workflow for configure, build, executable smoke run, and repository validation.

## Evidence Target

Provisioning path for wireless and wired building devices with retained configuration.

## Remote

Intended public repository: https://github.com/rheslar1/nrf52840-bacnet-field-node

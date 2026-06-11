# nRF52840 BACnet Field Node

C++17 embedded systems project for a battery-aware nRF52840 field node that maps room telemetry and retained setpoints into a BACnet-oriented object model.

The repository is intentionally host-buildable first. The current implementation models the field-node domain, commissioning flow, EEPROM-style retained configuration, BACnet object mapping, setpoint safety policy, alarm evaluation, telemetry-frame serialization, and battery-runtime estimation without requiring Nordic hardware on day one.

Intended remote:

```text
git@github.com:rheslar1/nrf52840-bacnet-field-node.git
```

## What This Project Demonstrates

- C++17 embedded application structure.
- C++ design patterns applied to a practical field-device problem.
- SOLID design principles in a small codebase.
- BACnet object modeling for building automation.
- Persistent setpoint/configuration storage abstraction.
- Commissioning workflow for field deployment evidence.
- Active alarm evaluation for comfort drift, battery, humidity, and radio link state.
- Serialized telemetry frame ready for a gateway or BMS bridge.
- Battery-aware telemetry and low-power runtime estimation.
- Host-side CMake, CTest, and GitHub Actions validation.
- CI, static analysis, and GitHub Pages deployment automation.

## Current Implementation

```text
include/field_node/FieldNode.hpp    Public C++17 domain API
src/FieldNode.cpp                   Field-node implementation
src/main.cpp                        CLI simulator for commissioning and telemetry
tests/FieldNodeTests.cpp            Host-side C++ validation tests
docs/                              Detailed engineering documentation
examples/                          Sample commissioning output
```

## Diagrams

| Diagram | PNG | Editable Draw.io |
| --- | --- | --- |
| Field node system architecture | [PNG](docs/diagrams/field-node-system.png) | [Draw.io](docs/diagrams/field-node-system.drawio) |
| C++17 SOLID and pattern map | [PNG](docs/diagrams/cpp-solid-patterns.png) | [Draw.io](docs/diagrams/cpp-solid-patterns.drawio) |
| Commissioning and storage flow | [PNG](docs/diagrams/commissioning-storage-flow.png) | [Draw.io](docs/diagrams/commissioning-storage-flow.drawio) |

## Automation

GitHub Actions runs CMake, CTest, CLI smoke tests, `cppcheck`, and `clang-tidy`. Pushes to `main` deploy a generated static evidence site to GitHub Pages after those gates pass.

See [docs/ci-static-analysis-and-deploy.md](docs/ci-static-analysis-and-deploy.md).

## Quick Start

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/nrf52840_bacnet_field_node --status
./build/nrf52840_bacnet_field_node --objects
./build/nrf52840_bacnet_field_node --alarms
./build/nrf52840_bacnet_field_node --telemetry 42
```

Useful simulator commands:

```bash
./build/nrf52840_bacnet_field_node --commission 700001 Tower-B Server-Room
./build/nrf52840_bacnet_field_node --sample 24.2 52 2210 0
./build/nrf52840_bacnet_field_node --setpoint 90
./build/nrf52840_bacnet_field_node --save field-node-eeprom.txt
./build/nrf52840_bacnet_field_node --load field-node-eeprom.txt
```

## C++17 Design Patterns

| Pattern | Where | Why |
| --- | --- | --- |
| Strategy | `ISetpointPolicy`, `ClampedSetpointPolicy` | Setpoint write policy can change without changing `FieldNode`. |
| Repository / Adapter | `IConfigStorage`, `FileConfigStorage` | Host file persistence can be replaced by nRF52 flash/FDS/EEPROM storage. |
| Mapper | `BacnetObjectMapper` | BACnet object generation is isolated from device state ownership. |
| Evaluator | `AlarmEvaluator` | Alarm thresholds are isolated from sampling, storage, and CLI concerns. |
| Service Object | `CommissioningService` | Provisioning and lock rules are separated from CLI and storage. |
| Facade / Aggregate Root | `FieldNode` | A single domain entry point coordinates config, samples, objects, and reports. |

## SOLID Principles

| Principle | Implementation |
| --- | --- |
| Single Responsibility | Battery estimation, alarm evaluation, BACnet mapping, commissioning, storage, and setpoint policy are separate types. |
| Open/Closed | New storage backends or setpoint policies can be added through interfaces. |
| Liskov Substitution | `IConfigStorage` and `ISetpointPolicy` implementations are replaceable by contract. |
| Interface Segregation | Storage and setpoint policies are small, focused interfaces. |
| Dependency Inversion | `FieldNode` depends on abstractions for setpoint behavior and storage. |

## BACnet Object Model

The host model exposes nine BACnet-style objects:

- Device object for the node identity.
- Writable occupied and unoccupied setpoint analog values.
- Temperature, humidity, and battery-voltage analog inputs.
- Occupancy binary input.
- Low-battery binary value.
- Commissioning-state multi-state value.

See [docs/bacnet-object-map.md](docs/bacnet-object-map.md).

## Alarm and Telemetry Model

`AlarmEvaluator` produces active alarms with `normal`, `advisory`, `warning`, and `critical` priority levels. The current rules cover:

- battery reserve below warning and critical thresholds,
- occupied or unoccupied comfort drift from the active setpoint,
- humidity outside preferred and warning bands,
- weak BLE RSSI during commissioning or periodic reporting.

`FieldNode::telemetryPayload()` serializes the current sample, active alarms, highest priority, runtime estimate, and BACnet object table into a deterministic JSON-style payload for gateway integration evidence.

## Hardware Path

The current host implementation is ready to map onto nRF52840 firmware layers:

- Replace `FileConfigStorage` with Nordic FDS, flash page storage, or external EEPROM.
- Replace CLI sensor samples with SAADC/I2C/SPI/BLE sensor adapters.
- Connect `BacnetObjectMapper` output to BACnet MS/TP, BACnet/IP gateway payloads, or a building gateway bridge.
- Publish `telemetryPayload()` output through the selected gateway transport after replacing host sensor simulation.
- Replace host CMake toolchain with a Zephyr or Nordic nRF Connect SDK board target.

## Validation Status

The current repo validates:

- C++17 configure/build.
- CTest execution.
- Setpoint clamping safety.
- Commissioning and lock workflow.
- BACnet object map generation.
- Low battery alarm mapping.
- Alarm priority evaluation.
- Telemetry frame serialization.
- Retained config save/load checksum validation.

See [docs/validation-plan.md](docs/validation-plan.md).

<!-- DESIGN_PACKAGE_START -->
## Detailed Design Package

This repository includes a structured design package for **nRF52840 BACnet Field Node**. The package captures the system boundary, runtime flow, hardware/software interfaces, validation strategy, and implementation roadmap.

| Artifact | Link |
| --- | --- |
| Design Index | [docs/design/README.md](docs/design/README.md) |
| System Design | [docs/design/system-design.md](docs/design/system-design.md) |
| Requirements | [docs/design/requirements.md](docs/design/requirements.md) |
| Interface Control | [docs/design/interface-control.md](docs/design/interface-control.md) |
| Runtime Design | [docs/design/runtime-design.md](docs/design/runtime-design.md) |
| Validation Plan | [docs/design/validation-plan.md](docs/design/validation-plan.md) |
| Implementation Roadmap | [docs/design/implementation-roadmap.md](docs/design/implementation-roadmap.md) |
| Draw.io UML | [docs/design/diagrams/system-design.drawio](docs/design/diagrams/system-design.drawio) |
| PNG UML | [docs/design/diagrams/system-design.png](docs/design/diagrams/system-design.png) |
<!-- DESIGN_PACKAGE_END -->

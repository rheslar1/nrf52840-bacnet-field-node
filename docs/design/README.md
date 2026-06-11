# nRF52840 BACnet Field Node Design Package

## Purpose

Battery-aware field device profile with persistent setpoint storage, commissioning evidence, and BACnet object mapping.

This design package turns the project concept into an implementation-ready engineering plan. It defines system boundaries, runtime responsibilities, interface contracts, validation evidence, and the staged path from host simulation to target hardware.

## Project Profile

| Field | Value |
| --- | --- |
| Portfolio category | Bare-metal device |
| Repository | `rheslar1/nrf52840-bacnet-field-node` |
| Primary stack | C++17, C++ Design Patterns, SOLID, nRF52840, C, EEPROM, BACnet, BLE-ready |
| Review proof point | Provisioning path for wireless and wired building devices with retained configuration. |

## Package Contents

| Document | Description |
| --- | --- |
| [System Design](system-design.md) | Architecture, runtime flow, state model, component responsibilities, and failure model. |
| [Requirements](requirements.md) | Functional, non-functional, safety, security, and traceability requirements. |
| [Interface Control](interface-control.md) | Hardware/software boundaries, adapter contracts, data contracts, and integration surfaces. |
| [Runtime Design](runtime-design.md) | C++17/SOLID module design, control loop, data processing, persistence or telemetry flow. |
| [Validation Plan](validation-plan.md) | Host, target, CI, hardware, security, and production validation strategy. |
| [Implementation Roadmap](implementation-roadmap.md) | Phased plan for building, testing, and hardening the project. |
| [System UML Draw.io](diagrams/system-design.drawio) | Editable diagram source. |
| [System UML PNG](diagrams/system-design.png) | Rendered diagram for quick review. |

## Design Principles

- Use C++17 value types for deterministic data contracts.
- Keep target-specific APIs behind narrow adapter interfaces.
- Apply SOLID boundaries so runtime orchestration does not depend directly on hardware drivers, transport libraries, or persistence details.
- Keep host-buildable tests aligned with target behavior through scripted adapters.
- Treat diagnostics, failure modes, and validation evidence as first-class deliverables.

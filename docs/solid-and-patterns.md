# C++17, Design Patterns, And SOLID Principles

This project is intentionally written as a C++17 embedded systems design exercise. It avoids a procedural single-file implementation so the code can demonstrate production-style design boundaries.

## C++17 Usage

- `std::array` for fixed-size BACnet MAC identity.
- `std::optional` for object lookup and storage load results.
- `std::unique_ptr` for injected setpoint policy ownership.
- `std::vector` for generated BACnet object views.
- RAII file handling through `std::ifstream` and `std::ofstream`.
- Strong enum classes for commissioning state and BACnet object type.

## Pattern Inventory

### Strategy

`ISetpointPolicy` defines the setpoint-write behavior.

Current strategy:

- `ClampedSetpointPolicy`

Future strategies:

- Occupancy-aware setpoint policy.
- Demand-response setpoint policy.
- BEMS-ai recommendation policy.

### Adapter / Repository

`IConfigStorage` hides storage details from the domain. `FileConfigStorage` is a host adapter that models EEPROM/FDS behavior with a local text file.

Future adapters:

- Nordic FDS storage adapter.
- External I2C EEPROM adapter.
- Zephyr settings subsystem adapter.

### Mapper

`BacnetObjectMapper` maps domain state into BACnet object values. This keeps protocol presentation separate from state mutation.

### Service Object

`CommissioningService` owns provisioning and lock rules. It prevents `FieldNode` from becoming a large object with every responsibility.

### Facade

`FieldNode` exposes a focused API for CLI, tests, and future firmware tasks.

## SOLID Matrix

| Principle | Project Evidence |
| --- | --- |
| Single Responsibility | Each class has one reason to change: storage, mapping, policy, battery, commissioning, or aggregate coordination. |
| Open/Closed | New setpoint and storage behavior can be added without editing `FieldNode` internals. |
| Liskov Substitution | Any `ISetpointPolicy` or `IConfigStorage` implementation can replace the current one if it honors the interface contract. |
| Interface Segregation | Interfaces are narrow and do not force implementations to support unrelated behavior. |
| Dependency Inversion | Domain behavior depends on abstractions where variation is expected. |

## Why This Matters For Embedded Work

Embedded projects often become difficult to test because hardware access, protocol mapping, configuration, and control rules get mixed into one file. This repo keeps those concerns separated so:

- host tests can run without a board,
- hardware adapters can be swapped in later,
- protocol mapping can be inspected independently,
- commissioning behavior can be validated before deployment,
- safety rules remain visible.

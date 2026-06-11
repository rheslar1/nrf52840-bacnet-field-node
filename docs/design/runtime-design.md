# nRF52840 BACnet Field Node Runtime Design

## Module Structure

| Module | Responsibility |
| --- | --- |
| `app` | CLI, startup, argument parsing, and process status. |
| `domain` | Project-specific state, calculations, and decisions. |
| `adapters` | Hardware, OS, transport, persistence, model, or simulator integration. |
| `policy` | Safety and readiness gates. |
| `reporting` | Human-readable reports, telemetry, logs, or persistence statements. |
| `tests` | Scripted scenarios and edge-case checks. |

## C++17 Design

Recommended implementation style:

- Use plain structs for input, state, decisions, and issues.
- Use strong enums for scenario, state, severity, and command types.
- Use `std::optional` for recoverable missing values.
- Use `std::variant` only when the domain has a real closed set of alternatives.
- Prefer explicit dependencies passed to constructors.
- Keep ownership simple with values, references, and `std::unique_ptr` for polymorphic strategies.

## Design Patterns

| Pattern | Use |
| --- | --- |
| Strategy | Swap processors, validators, model engines, update policies, or control algorithms. |
| Adapter | Hide target-specific APIs behind stable project interfaces. |
| Facade | Provide a small runtime object that executes one complete scenario or cycle. |
| Composite | Combine multiple validation rules into one policy gate. |
| Repository/Sink | Isolate persistence, telemetry, or evidence output. |

## Nominal Sequence

1. Build a runtime profile for `nrf52840-bacnet-field-node`.
2. Construct adapters and policies through dependency injection.
3. Collect scripted or target-backed input.
4. Convert input into domain state.
5. Evaluate policy gates.
6. Emit accepted output or rejected diagnostic evidence.
7. Return deterministic status for CI and automation.

## Project-Specific Focus

Battery-aware field device profile with persistent setpoint storage, commissioning evidence, and BACnet object mapping.

The runtime must make this focus measurable through logs, reports, tests, diagrams, or hardware captures. The proof point for review is: Provisioning path for wireless and wired building devices with retained configuration.

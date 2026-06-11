# nRF52840 BACnet Field Node Requirements

## Functional Requirements

| ID | Requirement | Verification |
| --- | --- | --- |
| FR-001 | Provide a deterministic host-buildable runtime for nRF52840 BACnet Field Node. | CMake/CTest or equivalent CI. |
| FR-002 | Implement the project focus: Battery-aware field device profile with persistent setpoint storage, commissioning evidence, and BACnet object mapping. | Scenario tests and documentation review. |
| FR-003 | Separate orchestration from hardware, OS, model, transport, and persistence adapters. | Code/design review. |
| FR-004 | Emit stable issue codes for rejected and degraded paths. | Failure scenario tests. |
| FR-005 | Produce reviewer-visible evidence: telemetry, logs, reports, SQL, traces, or command output. | Artifact inspection. |

## Non-Functional Requirements

| ID | Requirement | Target |
| --- | --- | --- |
| NFR-001 | Use C++17 for core runtime implementation. | Compiler configuration. |
| NFR-002 | Follow SOLID design boundaries and narrow interfaces. | Architecture review. |
| NFR-003 | Keep tests runnable without target hardware. | CI runner. |
| NFR-004 | Keep scenario outputs deterministic. | Golden scenario checks. |
| NFR-005 | Maintain design docs and UML assets in the repo. | Repository review. |

## Safety And Reliability Requirements

| ID | Requirement |
| --- | --- |
| SR-001 | Incomplete or unsafe input shall be rejected before side effects. |
| SR-002 | Health checks shall be visible in diagnostics. |
| SR-003 | Versioned metadata shall be used for firmware, model, protocol, or schema when applicable. |
| SR-004 | Failure paths shall preserve enough evidence to reproduce the issue. |

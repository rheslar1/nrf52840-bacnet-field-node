# Commissioning Guide

Commissioning assigns the field node a building identity, room identity, BACnet device instance, BACnet MAC identity, and BLE provisioning address.

## Host Simulator Flow

```bash
cmake -S . -B build
cmake --build build
./build/nrf52840_bacnet_field_node --commission 700001 Tower-B Server-Room
```

Expected evidence:

```text
device_instance=700001
zone=Tower-B
room=Server-Room
commission_state=provisioned
```

## Lock Flow

After provisioning succeeds, the node can be locked:

```bash
./build/nrf52840_bacnet_field_node --lock
```

In code, a locked node rejects further provisioning attempts.

## Retained Config Save/Load

```bash
./build/nrf52840_bacnet_field_node --save field-node-eeprom.txt
./build/nrf52840_bacnet_field_node --load field-node-eeprom.txt
```

The host file models EEPROM/FDS behavior. The saved config includes a checksum. Load fails if required values are missing or the checksum does not match.

## Hardware Mapping

| Host Simulator Step | nRF52840 Firmware Step |
| --- | --- |
| CLI `--commission` | BLE provisioning characteristic write |
| `FileConfigStorage` | Nordic FDS / flash / external EEPROM |
| CLI report | BLE diagnostic read or UART shell |
| CTest lock test | Host-side rule test plus HIL provisioning test |

## Commissioning Evidence To Capture

- Terminal output from `--commission`.
- Saved retained config file.
- BACnet object table after provisioning.
- Board photo with label matching zone/room.
- BLE provisioning packet or GATT screenshot after hardware integration.

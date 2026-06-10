#include "field_node/FieldNode.hpp"

#include <cassert>
#include <cstdio>
#include <string>

namespace {

constexpr std::array<std::uint8_t, field_node::kBacnetMacLength> kCommissionedMac{{
  0x52u, 0x84u, 0x40u, 0x10u, 0x00u, 0x01u, 0xBAu, 0xC0u
}};

void testDefaultConfigIsValid() {
  const field_node::FieldNode node;
  const auto validation = node.validate();

  assert(validation.ok);
  assert(node.config().deviceInstance == 528400u);
  assert(node.bacnetObjects().size() == 9u);
}

void testSetpointPolicyClampsWrites() {
  field_node::FieldNode node;

  node.writeOccupiedSetpoint(100.0);
  assert(node.config().occupiedSetpointF == node.config().maxSetpointF);

  node.writeOccupiedSetpoint(40.0);
  assert(node.config().occupiedSetpointF == node.config().minSetpointF);
}

void testCommissioningAndLock() {
  field_node::FieldNode node;
  auto result = node.provision(700001u, "Tower-B", "Server-Room", kCommissionedMac, "D6:2C:40:10:BA:C0");

  assert(result.ok);
  assert(node.config().commissionState == field_node::CommissionState::Provisioned);
  assert(node.config().zoneLabel == "Tower-B");

  result = node.lockCommissioning();
  assert(result.ok);
  assert(node.config().commissionState == field_node::CommissionState::Locked);

  result = node.provision(700002u, "Other", "Room", kCommissionedMac, "D6:2C:40:10:BA:C1");
  assert(!result.ok);
}

void testSensorSamplesAndObjectMap() {
  field_node::FieldNode node;

  node.applySensorSample({24.2, 52.0, 2210u, false, -70});

  assert(node.lowBatteryAlarm());
  const auto batteryObject = node.findObject(field_node::BacnetObjectType::BinaryValue, node.config().objectBaseInstance + 7u);
  assert(batteryObject.has_value());
  assert(batteryObject->presentValue == 1.0);
}

void testStorageRoundTrip() {
  const std::string path = "field-node-test-eeprom.txt";
  field_node::FieldNode node;
  std::string error;

  auto result = node.provision(700010u, "Floor-2", "Engineering-Lab", kCommissionedMac, "D6:2C:40:10:BA:C2");
  assert(result.ok);
  node.writeOccupiedSetpoint(73.5);

  const field_node::FileConfigStorage storage(path);
  assert(node.save(storage, error));

  field_node::FieldNode restored;
  assert(restored.load(storage, error));
  assert(restored.config().deviceInstance == 700010u);
  assert(restored.config().occupiedSetpointF == 73.5);
  assert(restored.config().zoneLabel == "Floor-2");

  std::remove(path.c_str());
}

}  // namespace

int main() {
  testDefaultConfigIsValid();
  testSetpointPolicyClampsWrites();
  testCommissioningAndLock();
  testSensorSamplesAndObjectMap();
  testStorageRoundTrip();
  return 0;
}

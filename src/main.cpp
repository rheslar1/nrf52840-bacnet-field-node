#include "field_node/FieldNode.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr std::array<std::uint8_t, field_node::kBacnetMacLength> kDemoMac{{
  0x52u, 0x84u, 0x40u, 0x10u, 0x00u, 0x01u, 0xBAu, 0xC0u
}};

void printUsage(const char* programName) {
  std::cout << "Usage: " << programName << " [command]\n\n"
            << "Commands:\n"
            << "  --status                         Print commissioning and battery status\n"
            << "  --objects                        Print BACnet object map\n"
            << "  --alarms                         Print active alarm evaluation\n"
            << "  --telemetry SEQUENCE             Print serialized telemetry frame\n"
            << "  --sample TEMP HUMIDITY MV OCC    Apply a simulated sensor sample\n"
            << "  --setpoint DEG_F                 Clamp and write occupied setpoint\n"
            << "  --commission INSTANCE ZONE ROOM  Provision BACnet instance and labels\n"
            << "  --lock                           Lock commissioning after provisioning\n"
            << "  --save PATH                      Save retained config to EEPROM image\n"
            << "  --load PATH                      Load retained config from EEPROM image\n"
            << "  --help                           Show this help text\n";
}

void printObjects(const field_node::FieldNode& node) {
  const auto objects = node.bacnetObjects();
  std::cout << "BACnet object map (" << objects.size() << " objects)\n";
  std::cout << "type,instance,name,value,writable,units,description\n";

  for (const auto& object : objects) {
    std::cout << field_node::toString(object.type) << ','
              << object.instance << ','
              << object.name << ','
              << object.presentValue << ','
              << (object.writable ? "yes" : "no") << ','
              << object.units << ','
              << object.description << '\n';
  }
}

void printAlarms(const field_node::FieldNode& node) {
  const auto alarms = node.alarms();
  std::cout << "highest_priority=" << field_node::toString(node.highestAlarmPriority()) << '\n';
  std::cout << "active_alarm_count=" << alarms.size() << '\n';
  std::cout << "priority,code,measured,threshold,message\n";

  for (const auto& alarm : alarms) {
    std::cout << field_node::toString(alarm.priority) << ','
              << alarm.code << ','
              << alarm.measuredValue << ','
              << alarm.thresholdValue << ','
              << alarm.message << '\n';
  }
}

int fail(const std::string& message) {
  std::cerr << "field-node error: " << message << '\n';
  return 1;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    field_node::FieldNode node;

    if (argc == 1 || std::string(argv[1]) == "--status") {
      std::cout << node.commissioningReport();
      return 0;
    }

    const std::string command = argv[1];

    if (command == "--help") {
      printUsage(argv[0]);
      return 0;
    }

    if (command == "--objects") {
      printObjects(node);
      return 0;
    }

    if (command == "--alarms") {
      printAlarms(node);
      return 0;
    }

    if (command == "--telemetry") {
      const std::uint32_t sequence = argc >= 3 ? static_cast<std::uint32_t>(std::stoul(argv[2])) : 1u;
      std::cout << node.telemetryPayload(sequence) << '\n';
      return 0;
    }

    if (command == "--sample") {
      if (argc < 6) {
        printUsage(argv[0]);
        return 1;
      }

      node.applySensorSample({
        std::stod(argv[2]),
        std::stod(argv[3]),
        static_cast<std::uint16_t>(std::stoul(argv[4])),
        std::stoi(argv[5]) != 0,
        -61
      });
      std::cout << node.commissioningReport();
      return 0;
    }

    if (command == "--setpoint") {
      if (argc < 3) {
        printUsage(argv[0]);
        return 1;
      }

      node.writeOccupiedSetpoint(std::stod(argv[2]));
      std::cout << "occupied_setpoint_f=" << node.config().occupiedSetpointF << '\n';
      return 0;
    }

    if (command == "--commission") {
      if (argc < 5) {
        printUsage(argv[0]);
        return 1;
      }

      const auto result = node.provision(
        static_cast<std::uint32_t>(std::stoul(argv[2])),
        argv[3],
        argv[4],
        kDemoMac,
        "D6:2C:40:10:BA:C0"
      );
      if (!result.ok) {
        return fail(result.message);
      }
      std::cout << node.commissioningReport();
      return 0;
    }

    if (command == "--lock") {
      auto result = node.provision(528401u, "Tower-B", "Server-Room", kDemoMac, "D6:2C:40:10:BA:C0");
      if (!result.ok) {
        return fail(result.message);
      }
      result = node.lockCommissioning();
      if (!result.ok) {
        return fail(result.message);
      }
      std::cout << node.commissioningReport();
      return 0;
    }

    if (command == "--save") {
      if (argc < 3) {
        printUsage(argv[0]);
        return 1;
      }

      const field_node::FileConfigStorage storage(argv[2]);
      std::string error;
      if (!node.save(storage, error)) {
        return fail(error);
      }
      std::cout << "saved=" << argv[2] << '\n';
      return 0;
    }

    if (command == "--load") {
      if (argc < 3) {
        printUsage(argv[0]);
        return 1;
      }

      const field_node::FileConfigStorage storage(argv[2]);
      std::string error;
      if (!node.load(storage, error)) {
        return fail(error);
      }
      std::cout << node.commissioningReport();
      return 0;
    }

    printUsage(argv[0]);
    return 1;
  } catch (const std::exception& exception) {
    return fail(exception.what());
  }
}

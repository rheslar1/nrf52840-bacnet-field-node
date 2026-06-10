#include "field_node/FieldNode.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace field_node {
namespace {

std::string checksumPayload(const RetainedConfig& config) {
  std::ostringstream stream;
  stream << config.magic << '|'
         << config.version << '|'
         << config.deviceInstance << '|'
         << config.vendorId << '|'
         << config.objectBaseInstance << '|'
         << std::fixed << std::setprecision(2)
         << config.minSetpointF << '|'
         << config.maxSetpointF << '|'
         << config.occupiedSetpointF << '|'
         << config.unoccupiedSetpointF << '|'
         << config.txIntervalSeconds << '|'
         << formatMac(config.bacnetMac) << '|'
         << config.bleAddress << '|'
         << config.zoneLabel << '|'
         << config.roomLabel << '|'
         << toString(config.commissionState);
  return stream.str();
}

std::string encodeCommissionState(CommissionState state) {
  return toString(state);
}

CommissionState decodeCommissionState(const std::string& value) {
  if (value == "factory") {
    return CommissionState::Factory;
  }
  if (value == "provisioned") {
    return CommissionState::Provisioned;
  }
  if (value == "locked") {
    return CommissionState::Locked;
  }
  throw std::invalid_argument("unknown commission state: " + value);
}

std::array<std::uint8_t, kBacnetMacLength> parseMac(const std::string& text) {
  std::array<std::uint8_t, kBacnetMacLength> mac{};
  std::stringstream stream(text);
  std::string segment;
  std::size_t index = 0u;

  while (std::getline(stream, segment, ':')) {
    if (index >= mac.size()) {
      throw std::invalid_argument("too many MAC segments");
    }
    mac[index++] = static_cast<std::uint8_t>(std::stoul(segment, nullptr, 16));
  }

  if (index != mac.size()) {
    throw std::invalid_argument("not enough MAC segments");
  }

  return mac;
}

double clamp(double value, double minValue, double maxValue) {
  return std::max(minValue, std::min(value, maxValue));
}

bool nonEmpty(const std::string& value) {
  return !value.empty();
}

}  // namespace

FileConfigStorage::FileConfigStorage(std::string path) : path_(std::move(path)) {}

bool FileConfigStorage::save(const RetainedConfig& config, std::string& error) const {
  RetainedConfig copy = config;
  refreshChecksum(copy);

  std::ofstream file(path_, std::ios::trunc);
  if (!file) {
    error = "could not open config file for writing: " + path_;
    return false;
  }

  file << "magic=" << copy.magic << '\n'
       << "version=" << copy.version << '\n'
       << "device_instance=" << copy.deviceInstance << '\n'
       << "vendor_id=" << copy.vendorId << '\n'
       << "object_base_instance=" << copy.objectBaseInstance << '\n'
       << "min_setpoint_f=" << copy.minSetpointF << '\n'
       << "max_setpoint_f=" << copy.maxSetpointF << '\n'
       << "occupied_setpoint_f=" << copy.occupiedSetpointF << '\n'
       << "unoccupied_setpoint_f=" << copy.unoccupiedSetpointF << '\n'
       << "tx_interval_seconds=" << copy.txIntervalSeconds << '\n'
       << "bacnet_mac=" << formatMac(copy.bacnetMac) << '\n'
       << "ble_address=" << copy.bleAddress << '\n'
       << "zone_label=" << copy.zoneLabel << '\n'
       << "room_label=" << copy.roomLabel << '\n'
       << "commission_state=" << encodeCommissionState(copy.commissionState) << '\n'
       << "checksum=" << copy.checksum << '\n';

  if (!file) {
    error = "failed while writing config file: " + path_;
    return false;
  }

  error.clear();
  return true;
}

std::optional<RetainedConfig> FileConfigStorage::load(std::string& error) const {
  std::ifstream file(path_);
  RetainedConfig config{};
  std::string line;

  if (!file) {
    error = "could not open config file for reading: " + path_;
    return std::nullopt;
  }

  try {
    while (std::getline(file, line)) {
      const auto delimiter = line.find('=');
      if (delimiter == std::string::npos) {
        continue;
      }

      const std::string key = line.substr(0, delimiter);
      const std::string value = line.substr(delimiter + 1u);

      if (key == "magic") {
        config.magic = static_cast<std::uint32_t>(std::stoul(value));
      } else if (key == "version") {
        config.version = static_cast<std::uint16_t>(std::stoul(value));
      } else if (key == "device_instance") {
        config.deviceInstance = static_cast<std::uint32_t>(std::stoul(value));
      } else if (key == "vendor_id") {
        config.vendorId = static_cast<std::uint16_t>(std::stoul(value));
      } else if (key == "object_base_instance") {
        config.objectBaseInstance = static_cast<std::uint32_t>(std::stoul(value));
      } else if (key == "min_setpoint_f") {
        config.minSetpointF = std::stod(value);
      } else if (key == "max_setpoint_f") {
        config.maxSetpointF = std::stod(value);
      } else if (key == "occupied_setpoint_f") {
        config.occupiedSetpointF = std::stod(value);
      } else if (key == "unoccupied_setpoint_f") {
        config.unoccupiedSetpointF = std::stod(value);
      } else if (key == "tx_interval_seconds") {
        config.txIntervalSeconds = static_cast<std::uint16_t>(std::stoul(value));
      } else if (key == "bacnet_mac") {
        config.bacnetMac = parseMac(value);
      } else if (key == "ble_address") {
        config.bleAddress = value;
      } else if (key == "zone_label") {
        config.zoneLabel = value;
      } else if (key == "room_label") {
        config.roomLabel = value;
      } else if (key == "commission_state") {
        config.commissionState = decodeCommissionState(value);
      } else if (key == "checksum") {
        config.checksum = static_cast<std::uint32_t>(std::stoul(value));
      }
    }
  } catch (const std::exception& exception) {
    error = exception.what();
    return std::nullopt;
  }

  const ValidationResult validation = validateConfig(config);
  if (!validation.ok) {
    error = validation.message;
    return std::nullopt;
  }

  error.clear();
  return config;
}

double ClampedSetpointPolicy::apply(double requestedSetpointF, const RetainedConfig& config) const {
  return clamp(requestedSetpointF, config.minSetpointF, config.maxSetpointF);
}

double BatteryEstimator::percent(std::uint16_t batteryMillivolts) const {
  if (batteryMillivolts <= 2000u) {
    return 0.0;
  }
  if (batteryMillivolts >= 3200u) {
    return 100.0;
  }
  return (static_cast<double>(batteryMillivolts) - 2000.0) * 100.0 / 1200.0;
}

bool BatteryEstimator::lowBatteryAlarm(std::uint16_t batteryMillivolts) const {
  return batteryMillivolts < 2300u;
}

double BatteryEstimator::runtimeDays(const SensorSample& sample, const BatteryProfile& profile) const {
  if (profile.sampleIntervalSeconds == 0u) {
    return 0.0;
  }

  const double radioActiveSecondsPerSample = 0.075;
  const double samplesPerDay = 86400.0 / static_cast<double>(profile.sampleIntervalSeconds);
  const double activeHours = (samplesPerDay * radioActiveSecondsPerSample) / 3600.0;
  const double sleepHours = 24.0 - activeHours;
  const double dailyMah =
    (activeHours * profile.activeCurrentMa) +
    (sleepHours * profile.sleepCurrentUa / 1000.0);
  const double usableMah = static_cast<double>(profile.capacityMah) * percent(sample.batteryMillivolts) / 100.0;

  if (dailyMah <= std::numeric_limits<double>::epsilon()) {
    return 0.0;
  }

  return usableMah / dailyMah;
}

std::vector<BacnetObject> BacnetObjectMapper::map(
  const RetainedConfig& config,
  const SensorSample& sample,
  const BatteryEstimator& batteryEstimator
) const {
  const std::uint32_t base = config.objectBaseInstance;
  const double commissionValue =
    config.commissionState == CommissionState::Factory ? 1.0 :
    config.commissionState == CommissionState::Provisioned ? 2.0 : 3.0;

  return {
    {BacnetObjectType::Device, config.deviceInstance, "nRF52840 Field Node", "device", 1.0, false, "BACnet device object"},
    {BacnetObjectType::AnalogValue, base + 1u, "Occupied Setpoint", "degF", config.occupiedSetpointF, true, "Retained occupied comfort setpoint"},
    {BacnetObjectType::AnalogValue, base + 2u, "Unoccupied Setpoint", "degF", config.unoccupiedSetpointF, true, "Retained unoccupied comfort setpoint"},
    {BacnetObjectType::AnalogInput, base + 3u, "Zone Temperature", "degC", sample.temperatureC, false, "Measured local zone temperature"},
    {BacnetObjectType::AnalogInput, base + 4u, "Relative Humidity", "percent", sample.humidityPercent, false, "Measured local relative humidity"},
    {BacnetObjectType::AnalogInput, base + 5u, "Battery Voltage", "mV", static_cast<double>(sample.batteryMillivolts), false, "Battery voltage before radio transmit"},
    {BacnetObjectType::BinaryInput, base + 6u, "Occupancy", "boolean", sample.occupancyDetected ? 1.0 : 0.0, false, "Local occupancy input"},
    {BacnetObjectType::BinaryValue, base + 7u, "Low Battery Alarm", "boolean", batteryEstimator.lowBatteryAlarm(sample.batteryMillivolts) ? 1.0 : 0.0, false, "Computed battery alarm"},
    {BacnetObjectType::MultiStateValue, base + 8u, "Commissioning State", "state", commissionValue, false, "1=factory 2=provisioned 3=locked"}
  };
}

ValidationResult CommissioningService::provision(
  RetainedConfig& config,
  std::uint32_t deviceInstance,
  std::string zoneLabel,
  std::string roomLabel,
  std::array<std::uint8_t, kBacnetMacLength> bacnetMac,
  std::string bleAddress
) const {
  if (config.commissionState == CommissionState::Locked) {
    return {false, "commissioning is locked"};
  }
  if (deviceInstance == 0u || zoneLabel.empty() || roomLabel.empty() || bleAddress.empty()) {
    return {false, "device instance, zone, room, and BLE address are required"};
  }

  config.deviceInstance = deviceInstance;
  config.zoneLabel = std::move(zoneLabel);
  config.roomLabel = std::move(roomLabel);
  config.bacnetMac = bacnetMac;
  config.bleAddress = std::move(bleAddress);
  config.commissionState = CommissionState::Provisioned;
  refreshChecksum(config);

  return validateConfig(config);
}

ValidationResult CommissioningService::lock(RetainedConfig& config) const {
  if (config.commissionState == CommissionState::Factory) {
    return {false, "factory node must be provisioned before lock"};
  }

  config.commissionState = CommissionState::Locked;
  refreshChecksum(config);
  return validateConfig(config);
}

FieldNode::FieldNode(RetainedConfig config, std::unique_ptr<ISetpointPolicy> setpointPolicy)
  : config_(std::move(config)),
    sample_(defaultSample()),
    batteryProfile_(defaultBatteryProfile()),
    setpointPolicy_(std::move(setpointPolicy)) {
  if (!setpointPolicy_) {
    setpointPolicy_ = std::make_unique<ClampedSetpointPolicy>();
  }

  if (config_.checksum == 0u) {
    refreshChecksum(config_);
  }

  const ValidationResult validation = validateConfig(config_);
  if (!validation.ok) {
    throw std::invalid_argument(validation.message);
  }
}

RetainedConfig FieldNode::defaultConfig() {
  RetainedConfig config{};
  refreshChecksum(config);
  return config;
}

SensorSample FieldNode::defaultSample() {
  return {22.1, 45.0, 3000u, true, -62};
}

BatteryProfile FieldNode::defaultBatteryProfile() {
  return {2200u, 8.5, 7.0, 60u};
}

const RetainedConfig& FieldNode::config() const {
  return config_;
}

const SensorSample& FieldNode::sample() const {
  return sample_;
}

const BatteryProfile& FieldNode::batteryProfile() const {
  return batteryProfile_;
}

ValidationResult FieldNode::validate() const {
  return validateConfig(config_);
}

void FieldNode::applySensorSample(const SensorSample& sample) {
  if (sample.temperatureC < -40.0 || sample.temperatureC > 85.0) {
    throw std::out_of_range("temperature sample outside nRF field-node validation range");
  }
  if (sample.humidityPercent < 0.0 || sample.humidityPercent > 100.0) {
    throw std::out_of_range("humidity sample must be 0-100 percent");
  }
  if (sample.batteryMillivolts < 1500u || sample.batteryMillivolts > 3600u) {
    throw std::out_of_range("battery millivolt sample outside supported range");
  }

  sample_ = sample;
}

void FieldNode::writeOccupiedSetpoint(double requestedSetpointF) {
  config_.occupiedSetpointF = setpointPolicy_->apply(requestedSetpointF, config_);
  refreshChecksum(config_);
}

ValidationResult FieldNode::provision(
  std::uint32_t deviceInstance,
  std::string zoneLabel,
  std::string roomLabel,
  std::array<std::uint8_t, kBacnetMacLength> bacnetMac,
  std::string bleAddress
) {
  return commissioningService_.provision(
    config_,
    deviceInstance,
    std::move(zoneLabel),
    std::move(roomLabel),
    bacnetMac,
    std::move(bleAddress)
  );
}

ValidationResult FieldNode::lockCommissioning() {
  return commissioningService_.lock(config_);
}

std::vector<BacnetObject> FieldNode::bacnetObjects() const {
  return objectMapper_.map(config_, sample_, batteryEstimator_);
}

std::optional<BacnetObject> FieldNode::findObject(BacnetObjectType type, std::uint32_t instance) const {
  const auto objects = bacnetObjects();
  const auto found = std::find_if(objects.begin(), objects.end(), [type, instance](const BacnetObject& object) {
    return object.type == type && object.instance == instance;
  });

  if (found == objects.end()) {
    return std::nullopt;
  }

  return *found;
}

double FieldNode::batteryPercent() const {
  return batteryEstimator_.percent(sample_.batteryMillivolts);
}

double FieldNode::runtimeDays() const {
  return batteryEstimator_.runtimeDays(sample_, batteryProfile_);
}

bool FieldNode::lowBatteryAlarm() const {
  return batteryEstimator_.lowBatteryAlarm(sample_.batteryMillivolts);
}

std::string FieldNode::commissioningReport() const {
  std::ostringstream report;
  report << "device_instance=" << config_.deviceInstance << '\n'
         << "zone=" << config_.zoneLabel << '\n'
         << "room=" << config_.roomLabel << '\n'
         << "commission_state=" << toString(config_.commissionState) << '\n'
         << "ble_address=" << config_.bleAddress << '\n'
         << "bacnet_mac=" << formatMac(config_.bacnetMac) << '\n'
         << "occupied_setpoint_f=" << std::fixed << std::setprecision(1) << config_.occupiedSetpointF << '\n'
         << "battery_mv=" << sample_.batteryMillivolts << '\n'
         << "battery_percent=" << std::setprecision(1) << batteryPercent() << '\n'
         << "runtime_estimate_days=" << std::setprecision(1) << runtimeDays() << '\n'
         << "low_battery_alarm=" << (lowBatteryAlarm() ? "true" : "false") << '\n';
  return report.str();
}

bool FieldNode::save(const IConfigStorage& storage, std::string& error) const {
  return storage.save(config_, error);
}

bool FieldNode::load(const IConfigStorage& storage, std::string& error) {
  const auto loaded = storage.load(error);
  if (!loaded.has_value()) {
    return false;
  }

  config_ = *loaded;
  return true;
}

std::uint32_t checksum(const RetainedConfig& config) {
  const std::string payload = checksumPayload(config);
  std::uint32_t hash = 2166136261u;

  for (const unsigned char byte : payload) {
    hash ^= static_cast<std::uint32_t>(byte);
    hash *= 16777619u;
  }

  return hash;
}

void refreshChecksum(RetainedConfig& config) {
  config.checksum = checksum(config);
}

ValidationResult validateConfig(const RetainedConfig& config) {
  if (config.magic != kConfigMagic) {
    return {false, "invalid config magic"};
  }
  if (config.version != kConfigVersion) {
    return {false, "unsupported config version"};
  }
  if (config.deviceInstance == 0u || config.vendorId == 0u || config.objectBaseInstance == 0u) {
    return {false, "device instance, vendor id, and object base instance are required"};
  }
  if (config.minSetpointF >= config.maxSetpointF) {
    return {false, "minimum setpoint must be less than maximum setpoint"};
  }
  if (config.occupiedSetpointF < config.minSetpointF || config.occupiedSetpointF > config.maxSetpointF) {
    return {false, "occupied setpoint outside configured bounds"};
  }
  if (config.unoccupiedSetpointF < config.minSetpointF || config.unoccupiedSetpointF > config.maxSetpointF) {
    return {false, "unoccupied setpoint outside configured bounds"};
  }
  if (config.txIntervalSeconds < 10u) {
    return {false, "transmit interval must be at least 10 seconds"};
  }
  if (!nonEmpty(config.zoneLabel) || !nonEmpty(config.roomLabel) || !nonEmpty(config.bleAddress)) {
    return {false, "zone label, room label, and BLE address are required"};
  }
  if (config.checksum != checksum(config)) {
    return {false, "config checksum mismatch"};
  }

  return {true, "ok"};
}

std::string toString(CommissionState state) {
  switch (state) {
    case CommissionState::Factory:
      return "factory";
    case CommissionState::Provisioned:
      return "provisioned";
    case CommissionState::Locked:
      return "locked";
  }

  return "unknown";
}

std::string toString(BacnetObjectType type) {
  switch (type) {
    case BacnetObjectType::AnalogInput:
      return "analog-input";
    case BacnetObjectType::AnalogValue:
      return "analog-value";
    case BacnetObjectType::BinaryInput:
      return "binary-input";
    case BacnetObjectType::BinaryValue:
      return "binary-value";
    case BacnetObjectType::Device:
      return "device";
    case BacnetObjectType::MultiStateValue:
      return "multi-state-value";
  }

  return "unknown";
}

std::string formatMac(const std::array<std::uint8_t, kBacnetMacLength>& mac) {
  std::ostringstream stream;

  for (std::size_t index = 0u; index < mac.size(); ++index) {
    if (index > 0u) {
      stream << ':';
    }
    stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<unsigned int>(mac[index]);
  }

  return stream.str();
}

}  // namespace field_node

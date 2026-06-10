#ifndef FIELD_NODE_FIELD_NODE_HPP
#define FIELD_NODE_FIELD_NODE_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace field_node {

constexpr std::uint32_t kConfigMagic = 0x4E424143u;
constexpr std::uint16_t kConfigVersion = 1u;
constexpr std::size_t kBacnetMacLength = 8u;

enum class CommissionState {
  Factory,
  Provisioned,
  Locked
};

enum class BacnetObjectType {
  AnalogInput,
  AnalogValue,
  BinaryInput,
  BinaryValue,
  Device,
  MultiStateValue
};

struct BacnetObject {
  BacnetObjectType type;
  std::uint32_t instance;
  std::string name;
  std::string units;
  double presentValue;
  bool writable;
  std::string description;
};

struct SensorSample {
  double temperatureC;
  double humidityPercent;
  std::uint16_t batteryMillivolts;
  bool occupancyDetected;
  std::int8_t rssiDbm;
};

struct BatteryProfile {
  std::uint16_t capacityMah;
  double activeCurrentMa;
  double sleepCurrentUa;
  std::uint16_t sampleIntervalSeconds;
};

struct RetainedConfig {
  std::uint32_t magic = kConfigMagic;
  std::uint16_t version = kConfigVersion;
  std::uint32_t deviceInstance = 528400u;
  std::uint16_t vendorId = 999u;
  std::uint32_t objectBaseInstance = 1000u;
  double minSetpointF = 60.0;
  double maxSetpointF = 82.0;
  double occupiedSetpointF = 72.0;
  double unoccupiedSetpointF = 78.0;
  std::uint16_t txIntervalSeconds = 60u;
  std::array<std::uint8_t, kBacnetMacLength> bacnetMac{{0x52u, 0x84u, 0x00u, 0x01u, 0x00u, 0x00u, 0x00u, 0x01u}};
  std::string bleAddress = "D6:2C:40:01:00:01";
  std::string zoneLabel = "Uncommissioned Zone";
  std::string roomLabel = "Bench Fixture";
  CommissionState commissionState = CommissionState::Factory;
  std::uint32_t checksum = 0u;
};

struct ValidationResult {
  bool ok;
  std::string message;
};

class IConfigStorage {
 public:
  virtual ~IConfigStorage() = default;
  virtual bool save(const RetainedConfig& config, std::string& error) const = 0;
  virtual std::optional<RetainedConfig> load(std::string& error) const = 0;
};

class FileConfigStorage final : public IConfigStorage {
 public:
  explicit FileConfigStorage(std::string path);

  bool save(const RetainedConfig& config, std::string& error) const override;
  std::optional<RetainedConfig> load(std::string& error) const override;

 private:
  std::string path_;
};

class ISetpointPolicy {
 public:
  virtual ~ISetpointPolicy() = default;
  virtual double apply(double requestedSetpointF, const RetainedConfig& config) const = 0;
};

class ClampedSetpointPolicy final : public ISetpointPolicy {
 public:
  double apply(double requestedSetpointF, const RetainedConfig& config) const override;
};

class BatteryEstimator final {
 public:
  double percent(std::uint16_t batteryMillivolts) const;
  bool lowBatteryAlarm(std::uint16_t batteryMillivolts) const;
  double runtimeDays(const SensorSample& sample, const BatteryProfile& profile) const;
};

class BacnetObjectMapper final {
 public:
  std::vector<BacnetObject> map(
    const RetainedConfig& config,
    const SensorSample& sample,
    const BatteryEstimator& batteryEstimator
  ) const;
};

class CommissioningService final {
 public:
  ValidationResult provision(
    RetainedConfig& config,
    std::uint32_t deviceInstance,
    std::string zoneLabel,
    std::string roomLabel,
    std::array<std::uint8_t, kBacnetMacLength> bacnetMac,
    std::string bleAddress
  ) const;

  ValidationResult lock(RetainedConfig& config) const;
};

class FieldNode final {
 public:
  explicit FieldNode(
    RetainedConfig config = defaultConfig(),
    std::unique_ptr<ISetpointPolicy> setpointPolicy = std::make_unique<ClampedSetpointPolicy>()
  );

  static RetainedConfig defaultConfig();
  static SensorSample defaultSample();
  static BatteryProfile defaultBatteryProfile();

  const RetainedConfig& config() const;
  const SensorSample& sample() const;
  const BatteryProfile& batteryProfile() const;

  ValidationResult validate() const;
  void applySensorSample(const SensorSample& sample);
  void writeOccupiedSetpoint(double requestedSetpointF);
  ValidationResult provision(
    std::uint32_t deviceInstance,
    std::string zoneLabel,
    std::string roomLabel,
    std::array<std::uint8_t, kBacnetMacLength> bacnetMac,
    std::string bleAddress
  );
  ValidationResult lockCommissioning();

  std::vector<BacnetObject> bacnetObjects() const;
  std::optional<BacnetObject> findObject(BacnetObjectType type, std::uint32_t instance) const;
  double batteryPercent() const;
  double runtimeDays() const;
  bool lowBatteryAlarm() const;
  std::string commissioningReport() const;

  bool save(const IConfigStorage& storage, std::string& error) const;
  bool load(const IConfigStorage& storage, std::string& error);

 private:
  RetainedConfig config_;
  SensorSample sample_;
  BatteryProfile batteryProfile_;
  std::unique_ptr<ISetpointPolicy> setpointPolicy_;
  BatteryEstimator batteryEstimator_;
  BacnetObjectMapper objectMapper_;
  CommissioningService commissioningService_;
};

std::uint32_t checksum(const RetainedConfig& config);
void refreshChecksum(RetainedConfig& config);
ValidationResult validateConfig(const RetainedConfig& config);

std::string toString(CommissionState state);
std::string toString(BacnetObjectType type);
std::string formatMac(const std::array<std::uint8_t, kBacnetMacLength>& mac);

}  // namespace field_node

#endif

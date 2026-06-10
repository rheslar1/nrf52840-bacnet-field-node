#include <stdio.h>
#include <stddef.h>

typedef struct {
  const char *title;
  const char *summary;
  const char *evidence_target;
  const char *tags[8];
  size_t tag_count;
} project_profile_t;

static const project_profile_t profile = {
  "nRF52840 BACnet Field Node",
  "Battery-aware field device profile with persistent setpoint storage, commissioning evidence, and BACnet object mapping.",
  "Provisioning path for wireless and wired building devices with retained configuration.",
  {
  "nRF52840",
  "C",
  "EEPROM",
  "BACnet",
  "BLE-ready"
  },
  5u
};

int main(void) {
  printf("%s\n", profile.title);
  printf("Summary: %s\n", profile.summary);
  printf("Evidence target: %s\n", profile.evidence_target);
  printf("Stack:");

  for (size_t index = 0; index < profile.tag_count; ++index) {
    printf(" %s%s", profile.tags[index], index + 1u == profile.tag_count ? "" : ",");
  }

  printf("\n");
  return 0;
}

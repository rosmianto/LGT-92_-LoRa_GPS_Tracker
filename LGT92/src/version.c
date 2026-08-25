#include <version.h>

bool isFwVersionNewer(uint8_t major, uint8_t minor, uint8_t revision) {

  if (major > FW_VERSION_MAJOR) return true;
  if (major < FW_VERSION_MAJOR) return false;

  // Major version matches. Check minor version.
  if (minor > FW_VERSION_MINOR) return true;
  if (minor < FW_VERSION_MINOR) return false;

  // Minor version matches. Check revision.
  if (revision > FW_VERSION_REVISION) return true;
  if (revision < FW_VERSION_REVISION) return false;

  // Well, the entire version number is same. Not newer!
  return false;
}
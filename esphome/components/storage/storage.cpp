#include "storage.h"
#include "esphome/core/log.h"

namespace esphome {
namespace storage {

static const char *const TAG = "storage";

Storage::Storage() {}

bool Storage::mount_partition(std::string partition, std::string label) {
  esp_err_t ret = ESP_FAIL;

  return ret;
}

bool Storage::format_partition(std::string partition, std::string label) {
  esp_err_t ret = ESP_FAIL;

  return ret;
}

bool Storage::open_file(std::string filename, FILE **file) {
  esp_err_t ret = ESP_FAIL;

  return ret;
}

bool Storage::close_file(FILE *file) {
  esp_err_t ret = ESP_FAIL;

  return ret;
}

bool Storage::read_chunk(FILE *file, uint32_t offset, uint32_t size, char *buf) {
  char *ret;

  return ret;
}

bool Storage::write(FILE *file, uint32_t offset, uint32_t size) {
  esp_err_t ret = ESP_FAIL;

  return ret;
}

}  // namespace storage
}  // namespace esphome

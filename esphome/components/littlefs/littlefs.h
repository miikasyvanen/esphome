#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/storage/storage.h"
#include "esphome/core/helpers.h"

#include "esp_littlefs.h"

namespace esphome {
namespace littlefs {

class LittleFS : public esphome::storage::Storage {
 public:
  LittleFS();

  void setup();
  void loop();

  bool mount_partition(std::string partition, std::string label) override;
  bool format_partition(std::string partition, std::string label) override;
  bool open_file(std::string filename, FILE **file) override;
  bool close_file(FILE *file) override;
  bool read_chunk(FILE *file, uint32_t offset, uint32_t size, char *buf) override;
  bool write(FILE *file, uint32_t offset, uint32_t size) override;

 protected:
  LittleFS *littlefs_;
};

}  // namespace littlefs
}  // namespace esphome

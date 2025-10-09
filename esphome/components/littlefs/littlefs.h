#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/storage/storage.h"
#include "esphome/core/helpers.h"

#include "esp_littlefs.h"

namespace esphome {
namespace littlefs {

class LittleFS : public Component {
 public:
  LittleFS();

  void setup();
  void loop();

  bool mount_partition(std::string partition);
  bool format_partition(std::string partition);
  bool open_file(std::string filename, FILE *file);
  bool close_file(FILE *file);
  uint8_t read_chunk(FILE *file, uint32_t offset, uint32_t size);
  bool write(FILE *file, uint32_t offset, uint32_t size);

 protected:
  LittleFS *littlefs_;
};

}  // namespace littlefs
}  // namespace esphome

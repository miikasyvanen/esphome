#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "esphome/components/storage/storage.h"
#include "esphome/components/littlefs/littlefs.h"

namespace esphome {
namespace internalfs {

class InternalFS : public esphome::littlefs::LittleFS {
 public:
  InternalFS();

  // LittleFS *resource = new LittleFS();

  void setup();
  void loop();

  // void set_source_littlefs() { LittleFS *fs = new LittleFS(); }
  LittleFS *fs;
  bool mount_partition(std::string partition, std::string label);
  bool format_partition(std::string partition, std::string label);
  bool open_file(std::string filename, FILE **file);
  bool close_file(FILE *file);
  bool read_chunk(FILE *file, uint32_t offset, uint32_t size, char *buf);
  bool write(FILE *file, uint32_t offset, uint32_t size);

 protected:
  InternalFS *internalfs_;
};

}  // namespace internalfs
}  // namespace esphome

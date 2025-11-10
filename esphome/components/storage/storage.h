#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "esphome/core/time.h"

namespace esphome {
namespace storage {

#define SUB_STORAGE(name) \
 protected: \
  storage_::Storage *name##_storage_{nullptr}; \
\
 public: \
  void set_##name##_storage(storage_::Storage *s) { this->name##_storage_ = storage; }

class Storage : public Component {
 public:
  explicit Storage();

  virtual bool mount_partition(std::string partition, std::string label);
  virtual bool format_partition(std::string partition, std::string label);
  virtual bool open_file(std::string filename, FILE **file);
  virtual bool close_file(FILE *file);
  virtual bool read_chunk(FILE *file, uint32_t offset, uint32_t size, char *buf);
  virtual bool write(FILE *file, uint32_t offset, uint32_t size);

 protected:
};

}  // namespace storage
}  // namespace esphome

#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/log.h"
#include "esphome/core/time.h"

namespace esphome {
namespace filesystem {

#define SUB_FILESYSTEM(name) \
 protected: \
  filesystem_::Filesystem *name##_filesystem_{nullptr}; \
\
 public: \
  void set_##name##_filesystem(filesystem_::Filesystem *s) { this->name##_filesystem_ = filesystem; }

class Filesystem : public Component {
 public:
  explicit Filesystem();

 protected:
};

}  // namespace filesystem
}  // namespace esphome

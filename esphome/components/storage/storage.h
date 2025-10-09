#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace storage {

#define SUB_STORAGE(name) \
 protected: \
  storage_::Storage *name##_storage_{nullptr}; \
\
 public: \
  void set_##name##_storage(storage_::Storage *s) { this->name##_storage_ = s; }

class Storage : public EntityBase, public EntityBase_DeviceClass {
 public:
  explicit Storage();
  // void setup() override;
  // void loop() override;

 protected:
};

}  // namespace storage
}  // namespace esphome

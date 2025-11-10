#include "internalfs.h"
#include "esphome/core/log.h"

namespace esphome {
namespace internalfs {

static const char *const TAG = "InternalFS";

InternalFS::InternalFS() {}

void InternalFS::setup() { fs = new LittleFS(); }

void InternalFS::loop() {}

bool InternalFS::mount_partition(std::string partition, std::string label) {
  return fs->mount_partition(partition, label);
}

bool InternalFS::format_partition(std::string partition, std::string label) {
  return fs->format_partition(partition, label);
}

bool InternalFS::open_file(std::string filename, FILE **file) { return fs->open_file(filename, file); }

bool InternalFS::close_file(FILE *file) { return fs->close_file(file); }

bool InternalFS::read_chunk(FILE *file, uint32_t offset, uint32_t size, char *buf) {
  return fs->read_chunk(file, offset, size, buf);
}

bool InternalFS::write(FILE *file, uint32_t offset, uint32_t size) { return fs->write(file, offset, size); }

}  // namespace internalfs
}  // namespace esphome

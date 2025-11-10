#include "littlefs.h"
#include "esphome/core/log.h"

namespace esphome {
namespace littlefs {

static const char *const TAG = "LittleFS";

LittleFS::LittleFS() {}

void LittleFS::setup() {}

void LittleFS::loop() {}

bool LittleFS::mount_partition(std::string partition, std::string label) {
  esp_err_t ret = ESP_FAIL;

  const char *p_label = label.c_str();

  if (partition.at(0) != '/')
    partition.insert(0, 1, '/');
  const char *p_path = partition.c_str();

  if (esp_littlefs_mounted(p_label)) {
    ESP_LOGE(TAG, "LittleFS partition already mounted!");
  }

  // Initialize LittleFS
  ESP_LOGI(TAG, "Mounting LittleFS partition");

  ESP_LOGI(TAG, "Mounting LittleFS partition %s with label %s", p_path, p_label);

  esp_vfs_littlefs_conf_t conf = {
      .base_path = p_path,
      .partition_label = p_label,
      .format_if_mount_failed = false,
      .dont_mount = false,
  };

  ret = esp_vfs_littlefs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find LittleFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
    }
    return ret;
  }

  ESP_LOGI(TAG, "LittleFS mounted succesfully");

  size_t total = 0, used = 0;
  ret = esp_littlefs_info(conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  }

  return ret;
}

bool LittleFS::format_partition(std::string partition, std::string label) {
  esp_err_t ret = ESP_FAIL;

  const char *p_label = partition.c_str();
  partition.insert(0, 1, '/');
  const char *p_path = partition.c_str();

  return ret;
}

bool LittleFS::open_file(std::string filename, FILE **file) {
  esp_err_t ret = ESP_FAIL;

  // const char *fname = filename.c_str();

  uint8_t filename_len = strlen("/littlefs") + strlen(filename.c_str()) + 1;
  char fname[filename_len];
  // filename = (char*)malloc(filename_len);
  strcpy(fname, "/littlefs");
  strcat(fname, filename.c_str());

  ESP_LOGI(TAG, "Opening file: %s", fname);
  *file = fopen(fname, "r");
  if (file == NULL) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return ret;
  }
  ret = ESP_OK;

  return ret;
}

bool LittleFS::close_file(FILE *file) {
  esp_err_t ret = ESP_FAIL;

  fclose(file);

  return ret;
}

bool LittleFS::read_chunk(FILE *file, uint32_t offset, uint32_t size, char *buf) {
  esp_err_t ret = ESP_FAIL;

  // buf = (char *) malloc(sizeof(char) * size);
  fseek(file, 0, offset);
  int readsize = fread(buf, 1, size, file);

  return ret;
}

bool LittleFS::write(FILE *file, uint32_t offset, uint32_t size) {
  esp_err_t ret = ESP_FAIL;

  return ret;
}

}  // namespace littlefs
}  // namespace esphome

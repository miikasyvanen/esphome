#include "captive_portal.h"
#ifdef USE_CAPTIVE_PORTAL
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/wifi/wifi_component.h"
#include "captive_index.h"

#include "esphome/components/web_server/web_server.h"
// esphome::web_server::WebServer *web_server;

namespace esphome {
namespace captive_portal {

static const char *const TAG = "captive_portal";

void CaptivePortal::handle_captive_portal(AsyncWebServerRequest *request) {
#ifndef USE_ESP8266
  auto *response = request->beginResponse(200, "text/html", INDEX_GZ, sizeof(INDEX_GZ));
#else
  auto *response = request->beginResponse_P(200, "text/html", INDEX_GZ, sizeof(INDEX_GZ));
#endif
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void CaptivePortal::handle_config(AsyncWebServerRequest *request) {
  AsyncResponseStream *stream = request->beginResponseStream(F("application/json"));
  stream->addHeader(F("cache-control"), F("public, max-age=0, must-revalidate"));
#ifdef USE_ESP8266
  stream->print(F("{\"mac\":\""));
  stream->print(get_mac_address_pretty().c_str());
  stream->print(F("\",\"name\":\""));
  stream->print(App.get_name().c_str());
  stream->print(F("\",\"aps\":[{}"));
#else
  stream->printf(R"({"mac":"%s","name":"%s","aps":[{})", get_mac_address_pretty().c_str(), App.get_name().c_str());
#endif
  bool passive_scan = request->url() == "/" ? false : true;

  wifi::global_wifi_component->start_scanning(passive_scan);

  for (auto &scan : wifi::global_wifi_component->get_scan_result()) {
    if (scan.get_is_hidden())
      continue;

      // Assumes no " in ssid, possible unicode isses?
#ifdef USE_ESP8266
    stream->print(F(",{\"ssid\":\""));
    stream->print(scan.get_ssid().c_str());
    stream->print(F("\",\"rssi\":"));
    stream->print(scan.get_rssi());
    stream->print(F(",\"lock\":"));
    stream->print(scan.get_with_auth());
    stream->print(F("}"));
#else
    stream->printf(R"(,{"ssid":"%s","rssi":%d,"lock":%d})", scan.get_ssid().c_str(), scan.get_rssi(),
                   scan.get_with_auth());
#endif
  }
  stream->print(F("]}"));
  request->send(stream);
}
void CaptivePortal::handle_wifisave(AsyncWebServerRequest *request) {
  std::string ssid = request->arg("ssid").c_str();  // NOLINT(readability-redundant-string-cstr)
  std::string psk = request->arg("psk").c_str();    // NOLINT(readability-redundant-string-cstr)
  ESP_LOGI(TAG, "Requested WiFi Settings Change:");
  ESP_LOGI(TAG, "  SSID='%s'", ssid.c_str());
  ESP_LOGI(TAG, "  Password=" LOG_SECRET("'%s'"), psk.c_str());
  wifi::global_wifi_component->save_wifi_sta(ssid, psk);
  request->redirect(F("/?save"));
}

void CaptivePortal::handleRequest(AsyncWebServerRequest *req) {
  if (req->url() == this->portal_path_) {
    this->handle_captive_portal(req);
    return;
  } else if (req->url() == "/config.json") {
    this->handle_config(req);
    return;
  } else if (req->url() == "/wifisave") {
    this->handle_wifisave(req);
    return;
  }
}

void CaptivePortal::setup() {
  // Disable loop by default - will be enabled when captive portal starts
  this->disable_loop();
}
void CaptivePortal::start(const String portal_path) {
  ESP_LOGV(TAG, "Starting Captive Portal using path: %s", portal_path.c_str());
  this->portal_path_ = portal_path;
  this->base_->init();
  if (!this->initialized_) {
    this->base_->add_handler(this);
  }

  network::IPAddress ip = wifi::global_wifi_component->wifi_soft_ap_ip();

#ifdef USE_WIFI_AP
#ifdef USE_ESP_IDF
  // Create DNS server instance for ESP-IDF
  this->dns_server_ = make_unique<DNSServer>();
  this->dns_server_->start(ip);
#endif
#ifdef USE_ARDUINO
  this->dns_server_ = make_unique<DNSServer>();
  this->dns_server_->setErrorReplyCode(DNSReplyCode::NoError);
  this->dns_server_->start(53, F("*"), ip);
#endif
#endif  // USE_WIFI_AP
  this->initialized_ = true;
  this->active_ = true;

  // Enable loop() now that captive portal is active
  this->enable_loop();

  ESP_LOGV(TAG, "Captive portal started");
}
/*
void CaptivePortal::handleRequest(AsyncWebServerRequest *req) {
  if (req->url() == F("/config.json")) {
    this->handle_config(req);
    return;
  } else if (req->url() == F("/wifisave")) {
    this->handle_wifisave(req);
    return;
  }

  // All other requests get the captive portal page
  // This includes OS captive portal detection endpoints which will trigger
  // the captive portal when they don't receive their expected responses
  if (req->url() == F("/fallback")) {
#ifndef USE_ESP8266
    auto *response = req->beginResponse(200, F("text/html"), INDEX_GZ, sizeof(INDEX_GZ));
#else
    auto *response = req->beginResponse_P(200, F("text/html"), INDEX_GZ, sizeof(INDEX_GZ));
#endif
    response->addHeader(F("Content-Encoding"), F("gzip"));
    req->send(response);
  } else {
    web_server::global_web_server->handleRequest(req);
  }
}
*/
void CaptivePortal::end() {
  ESP_LOGV(TAG, "Ending Captive Portal...");

  this->active_ = false;
  this->base_->deinit();
#ifdef USE_ARDUINO
  this->dns_server_->stop();
  this->dns_server_ = nullptr;
#endif
}

CaptivePortal::CaptivePortal(web_server_base::WebServerBase *base) : base_(base) { global_captive_portal = this; }
float CaptivePortal::get_setup_priority() const {
  // Before WiFi
  return setup_priority::WIFI + 1.0f;
}
void CaptivePortal::dump_config() { ESP_LOGCONFIG(TAG, "Captive Portal:"); }

CaptivePortal *global_captive_portal = nullptr;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

}  // namespace captive_portal
}  // namespace esphome
#endif

#include "paj7620.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace paj7620 {

static const char *const TAG = "paj7620";

#define PAJ7620_WARNING_CHECK(func, warning) \
  if (!(func)) { \
    ESP_LOGW(TAG, warning); \
    this->status_set_warning(); \
    return; \
  }

#define PAJ7620_ERROR_CHECK(func) \
  if (!(func)) { \
    this->mark_failed(); \
    return; \
  }

#define PAJ7620_WRITE_BYTE(reg, value) PAJ7620_ERROR_CHECK(this->write_byte(reg, value));

#define PAJ7620_DELAY(value) \
  this->delay_req_ = value; \
  this->delay_start_ = millis();

void PAJ7620::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PAJ7620...");

  PAJ7620_WRITE_BYTE(0xFF, 0x00);

  uint8_t id00, id01;
  if (!this->read_byte(0x00, &id00) || !this->read_byte(0x01, &id01)) {  // ID registers
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  if (id00 != 0x20 || id01 != 0x76) {  // PAJ7620 all should have one of these IDs
    this->error_code_ = WRONG_ID;
    this->mark_failed();
    return;
  }

  // Load the registers data
  for (uint8_t i = 0; i < PAJ7620_INIT_REG_ARRAY_SIZE; i++)
    PAJ7620_WRITE_BYTE(paj7620_init_register_array[i][0], paj7620_init_register_array[i][1]);

  PAJ7620_WRITE_BYTE(PAJ7620_REG_BANK_SEL, 1);  // set to Bank1
  PAJ7620_WRITE_BYTE(0x65, this->report_mode_);
  PAJ7620_WRITE_BYTE(PAJ7620_REG_BANK_SEL, 0);  // set to Bank0
  ESP_LOGCONFIG(TAG, "Success...");
}

void PAJ7620::dump_config() {
  ESP_LOGCONFIG(TAG, "PAJ7620:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    switch (this->error_code_) {
      case COMMUNICATION_FAILED:
        ESP_LOGE(TAG, "Communication with PAJ7620 failed!");
        break;
      case WRONG_ID:
        ESP_LOGE(TAG, "PAJ7620 has invalid id!");
        break;
      default:
        ESP_LOGE(TAG, "Setting up PAJ7620 registers failed!");
        break;
    }
  }
}

void PAJ7620::loop() {
  if (this->delay_req_ == 0 || millis() - this->delay_start_ > this->delay_req_) {
    this->delay_req_ = 0;
    this->delay_start_ = 0;
    this->read_gesture_data_();
  }
}

float PAJ7620::get_setup_priority() const { return setup_priority::DATA; }

void PAJ7620::read_gesture_data_() {
  if (!this->is_gesture_enabled_())
    return;

  uint8_t gesture_code_h, gesture_code_l;
  PAJ7620_WARNING_CHECK(this->read_byte(PAJ7620_REG_RESULT_H, &gesture_code_h), "Reading gesture hi reg failed.");
  PAJ7620_WARNING_CHECK(this->read_byte(PAJ7620_REG_RESULT_L, &gesture_code_l), "Reading gesture low reg failed.");
  const uint16_t gesture_code = (gesture_code_h << 8) + gesture_code_l;
  if (gesture_code == 0)
    return;

  paj7620_gesture_t current_gesture = NONE;
  for (uint8_t i = UP; i < PAJ7620_GESTURE_COUNT; i++) {
    if (gesture_code == (1 << i)) {
      current_gesture = (paj7620_gesture_t) i;
      PAJ7620_WARNING_CHECK(this->read_byte(PAJ7620_REG_RESULT_H, &gesture_code_h), "Clearing gesture hi reg failed.");
      PAJ7620_WARNING_CHECK(this->read_byte(PAJ7620_REG_RESULT_L, &gesture_code_l), "Clearing gesture low reg failed.");
      break;
    }
  }

  switch (current_gesture) {
    case UP:
    case DOWN:
    case LEFT:
    case RIGHT:
      if (this->prev_gesture == current_gesture) {
        this->prev_gesture = NONE;
        this->report_gesture_(current_gesture);
      } else {
        PAJ7620_DELAY(this->gesture_entery_time_);
        this->prev_gesture = current_gesture;
      }
      break;

    case PUSH:
    case POLL:
      PAJ7620_DELAY(this->gesture_quit_time_);
    case CLOCKWISE:
    case ANTI_CLOCKWISE:
    case WAVE:
      this->report_gesture_(current_gesture);
      break;
    default:
      return;
  }
}

void PAJ7620::report_gesture_(paj7620_gesture_t gesture) {
#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *bin;
  switch (gesture) {
    case UP:
      bin = this->up_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture UP");
      break;

    case DOWN:
      bin = this->down_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture DOWN");
      break;

    case LEFT:
      bin = this->left_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture LEFT");
      break;

    case RIGHT:
      bin = this->right_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture RIGHT");
      break;

    case PUSH:
      bin = this->push_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture PUSH");
      break;

    case POLL:
      bin = this->poll_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture POLL");
      break;

    case CLOCKWISE:
      bin = this->clockwise_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture CLOCKWISE");
      break;

    case ANTI_CLOCKWISE:
      bin = this->anti_clockwise_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture ANTI_CLOCKWISE");
      break;

    case WAVE:
      bin = this->wave_direction_binary_sensor_;
      ESP_LOGD(TAG, "Got gesture WAVE");
      break;

    default:
      return;
  }

  if (bin != nullptr) {
    bin->publish_state(true);
    bin->publish_state(false);
  }
#endif
}

bool PAJ7620::is_gesture_enabled_() const {
#ifdef USE_BINARY_SENSOR
  return this->up_direction_binary_sensor_ != nullptr || this->left_direction_binary_sensor_ != nullptr ||
         this->down_direction_binary_sensor_ != nullptr || this->right_direction_binary_sensor_ != nullptr ||
         this->push_direction_binary_sensor_ != nullptr || this->poll_direction_binary_sensor_ != nullptr ||
         this->clockwise_direction_binary_sensor_ != nullptr ||
         this->anti_clockwise_direction_binary_sensor_ != nullptr || this->wave_direction_binary_sensor_ != nullptr;
#else
  return false;
#endif
}

}  // namespace paj7620
}  // namespace esphome

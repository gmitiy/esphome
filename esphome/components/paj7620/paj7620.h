#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

namespace esphome {
namespace paj7620 {

static const uint8_t paj7620_init_register_array[][2] = {
    // BANK 0
    {0xEF, 0x00},
    {0x37, 0x07},
    {0x38, 0x17},
    {0x39, 0x06},
    {0x42, 0x01},
    {0x46, 0x2D},
    {0x47, 0x0F},
    {0x48, 0x3C},
    {0x49, 0x00},
    {0x4A, 0x1E},
    {0x4C, 0x20},
    {0x51, 0x10},
    {0x5E, 0x10},
    {0x60, 0x27},
    {0x80, 0x42},
    {0x81, 0x44},
    {0x82, 0x04},
    {0x8B, 0x01},
    {0x90, 0x06},
    {0x95, 0x0A},
    {0x96, 0x0C},
    {0x97, 0x05},
    {0x9A, 0x14},
    {0x9C, 0x3F},
    {0xA5, 0x19},
    {0xCC, 0x19},
    {0xCD, 0x0B},
    {0xCE, 0x13},
    {0xCF, 0x64},
    {0xD0, 0x21},
    // BANK 1
    {0xEF, 0x01},
    {0x02, 0x0F},
    {0x03, 0x10},
    {0x04, 0x02},
    {0x25, 0x01},
    {0x27, 0x39},
    {0x28, 0x7F},
    {0x29, 0x08},
    {0x3E, 0xFF},
    {0x5E, 0x3D},
    {0x65, 0x96},
    {0x67, 0x97},
    {0x69, 0xCD},
    {0x6A, 0x01},
    {0x6D, 0x2C},
    {0x6E, 0x01},
    {0x72, 0x01},
    {0x73, 0x35},
    {0x77, 0x01},
    {0xEF, 0x00},
};
#define PAJ7620_INIT_REG_ARRAY_SIZE (sizeof(paj7620_init_register_array) / sizeof(paj7620_init_register_array[0]))

#define PAJ7620_REG_BANK_SEL 0xEF
#define PAJ7620_REG_RESULT_L 0x43
#define PAJ7620_REG_RESULT_H 0x44

#define PAJ7620_GESTURE_COUNT 9
typedef enum paj7620_gesture_type {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  PUSH,
  POLL,
  CLOCKWISE,
  ANTI_CLOCKWISE,
  WAVE,
  NONE
} paj7620_gesture_type_t;
typedef paj7620_gesture_type_t paj7620_gesture_t;

class PAJ7620 : public Component, public i2c::I2CDevice {
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(up_direction)
  SUB_BINARY_SENSOR(right_direction)
  SUB_BINARY_SENSOR(down_direction)
  SUB_BINARY_SENSOR(left_direction)
  SUB_BINARY_SENSOR(push_direction)
  SUB_BINARY_SENSOR(poll_direction)
  SUB_BINARY_SENSOR(clockwise_direction)
  SUB_BINARY_SENSOR(anti_clockwise_direction)
  SUB_BINARY_SENSOR(wave_direction)
#endif

 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void loop() override;

  void set_report_mode(uint8_t report_mode) { this->report_mode_ = report_mode; }
  void set_gesture_entery_time(int gesture_entery_time) { this->gesture_entery_time_ = (uint32_t) gesture_entery_time; }
  void set_gesture_quit_time(int gesture_quit_time) { this->gesture_quit_time_ = (uint32_t) gesture_quit_time; }

 protected:
  bool is_gesture_enabled_() const;
  void read_gesture_data_();
  void report_gesture_(paj7620_gesture_t gesture);

  uint8_t report_mode_;
  uint32_t gesture_entery_time_;
  uint32_t gesture_quit_time_;

  enum ErrorCode {
    NONE_ = 0,
    COMMUNICATION_FAILED,
    WRONG_ID,
  } error_code_{NONE_};

  uint32_t delay_req_{0};
  uint32_t delay_start_{0};
  paj7620_gesture_t prev_gesture{NONE};
};

}  // namespace paj7620
}  // namespace esphome

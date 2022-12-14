#include <Arduino.h>

typedef enum {
  PATTERN_NONE,
  PATTERN_HBEAT,
  PATTERN_ERR  
} led_patterns_type_t;

void handle_io_pattern(uint8_t pin, led_patterns_type_t target_pattern);
void begin_hspota();
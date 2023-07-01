void led_control(bool waiting, bool open, bool connecting) {
  if (waiting) {
    digitalWrite(blue_led_pin, 1);
  } else {
    digitalWrite(blue_led_pin, 0);
  }

  if (connecting) {
    digitalWrite(green_led_pin, 1);
    digitalWrite(red_led_pin, 1);
    return;
  }

  if (open) {
    digitalWrite(green_led_pin, 1);
    digitalWrite(red_led_pin, 0);
  } else {
    digitalWrite(green_led_pin, 0);
    digitalWrite(red_led_pin, 1);
  }
}
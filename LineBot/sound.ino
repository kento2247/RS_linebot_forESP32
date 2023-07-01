void scool_melody() {
  int ring_time = 200;
  int melody[] = {659, 698, 659, 698, 698, 880, 784, 784, 659, 784, 0};
  for (byte i = 0; i < sizeof(melody); i++) {
    ledcWriteTone(0, melody[i] / 2);
    delay(ring_time);
  }
}

void cure_music() {
  int ring_time = 200;
  int melody[] = {988, 0, 988, 0, 988, 831, 1319, 0};
  for (byte i = 0; i < sizeof(melody); i++) {
    ledcWriteTone(0, melody[i] / 2);
    delay(ring_time);
  }
}
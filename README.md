Simple project on ATiny13 + 0.96" OLED display (SSD1306 controller) + SS49E (hall sensor)
On startup it calibrates to zero, then shows ADC value (mean from 256 samples).
I didn't implement convertation values to Tesla/Gauss, because it requires floating point
operations which does not fit into ATiny13 memory.
I uses it to compare one magnet to another.
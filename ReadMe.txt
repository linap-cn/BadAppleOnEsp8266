程序在esp8266的RTOS SDK3.1上测试通过。
测试屏幕为ssd1306驱动的oled屏，分辨率128*64。
视频数据在BadApple.dat，分辨率60x45。
使用方法：
①将BadApple.dat烧录至0x10000。
②修改ssd1306.h里的SDA_PIN和SCL_PIN。
③编译，烧录。

ssd1306驱动代码来自：https://github.com/salara1960/esp_lora
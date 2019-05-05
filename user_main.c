#include <string.h>

#include <spi_flash.h>

#include "ssd1306.h"

#define DATA_ADDR 0x100000
#define DATA_LEN 837000

extern uint8_t buffer[128 * 64 / 8];
void app_main()
{
	ssd1306_init();
	ssd1306_on(true);
	while(1){
		ssd1306_clear();
		uint32_t len=0;
		uint8_t buf[360];
		while(len<DATA_LEN){
			spi_flash_read(DATA_ADDR+len,&buf,sizeof(buf));
			for(int j=0;j<6;++j){
				memcpy(buffer+j*128+256,buf+j*60,60);
				memcpy(buffer+j*128+320,buf+j*60,60);
			}
			ssd1306_draw();
			len+=360;
		}
	}
}

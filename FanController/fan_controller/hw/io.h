/* Autor: Ján Suèan <jan@jansucan.sk>
 * 
 * Zdrojove kody, ich casti a subory z nich vzniknute priamo alebo nepriamo
 * (objektove subory, Intel Hex, ...) prosim nepouzivajte komercne, ani ako
 * sucast komercnych diel. Vsetky ostatne pripady pouzitia su dovolene.
 *
 * Please don't use the source codes, their parts and files created from them
 * directly or indirectly, (object files, Intel Hex files, ...) for commercial
 * purposes, not even as a part of commercial products. All other use cases
 * are allowed.
 */

#ifndef IO_H_
#define IO_H_

#include <stdint.h>
#include <stdbool.h>

void io_init(void);

bool io_is_button_pressed(void);
void io_disable_fan_pwm(void);
bool io_is_fan_tachometer_set(void);
void io_on_red_led(void);
void io_off_red_led(void);
void io_on_green_led_0(void);
void io_set_green_led_0(uint8_t light);
void io_off_green_led_0(void);
void io_on_green_led_1(void);
void io_off_green_led_1(void);
void io_on_green_led_2(void);
void io_off_green_led_2(void);
void io_on_green_led_3(void);
void io_off_green_led_3(void);
void io_on_green_led_4(void);
void io_off_green_led_4(void);
void io_on_green_led_5(void);
void io_off_green_led_5(void);
void io_on_green_led_6(void);
void io_off_green_led_6(void);
void io_on_green_led_7(void);
void io_off_green_led_7(void);
void io_on_green_led_8(void);
void io_off_green_led_8(void);

#endif /* IO_H_ */
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

#ifndef LED_H_
#define LED_H_

#include <stdint.h>
#include <stdbool.h>

/** Pocet LED v stlpci zelenych LED pre indikaciu otacok. LED najviac nalavo nie je zapocitana. */
#define LED_GREEN_BAR_LED_COUNT    8U

void led_init(void);
void led_red_off(void);
void led_red_on(void);
void led_min_green_off(void);
void led_min_green_set(uint8_t light);
void led_min_green_on(void);
void led_green_bar_show_by_mask(uint8_t mask);
bool led_green_bar_will_be_changed_by_value(uint8_t value);
void led_green_bar_show_by_value(uint8_t value);

#endif /* LED_H_ */
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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdbool.h>
#include <stdint.h>

/** Kody modov zobrazenia otacok a pomocne hodnoty. */
enum display_type_codes {
	DISPLAY_TYPE_SET = 0, /**< Otacky nastavene na potenciometri. */
	DISPLAY_TYPE_REAL, /**< Realne otacky ventilatora. */
	DISPLAY_TYPE_SET_WITH_OFF, /**< Otacky nastavene na potenciometri s automatickym vypinanim zobrazenia. */
	DISPLAY_TYPE_REAL_WITH_OFF, /**< Realne otacky ventilatora s automatickym vypinanim zobrazenia. */
	DISPLAY_TYPE_COUNT, /**< Pocet kodov modov zobrazeni. */
	DISPLAY_TYPE_OVERRIDE, /**< Mod pre nastavenie otacok na konstantnu hodnotu sa nepocita medzi standardne mody. */
	DISPLAY_TYPE_NONE /**< Neznamy mod zobrazenia. */
};

/** Mod zobrazenia na ktory sa prepina, ked uz neexistuje dalsi mod (s vyssim cislom). */
#define DISPLAY_TYPE_FIRST    DISPLAY_TYPE_SET

void     config_disp_type_save(uint8_t disp_type);
uint8_t  config_disp_type_load(void);
bool     config_is_fan_max_rpm_pulse_time_init(void);
void     config_fan_max_rpm_pulse_time_save(uint16_t pulse_time);
uint16_t config_fan_max_rpm_pulse_time_load(void);
bool     config_is_fan_min_rpm_pulse_time_init(void);
void     config_fan_min_rpm_pulse_time_save(uint16_t pulse_time);
uint16_t config_fan_min_rpm_pulse_time_load(void);

#endif /* CONFIG_H_ */
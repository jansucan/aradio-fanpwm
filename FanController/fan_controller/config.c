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

#include <fan_controller/config.h>
#include <fan_controller/hw/eeprom.h>

/** Hodnota EEPROM ktora oznacuje neinizializovany bajt. */
#define CONFIG_UNINITIALIZED_VALUE   0xff

/** Hodnota EEPROM ktora oznacuje inizializovany bajt. */
#define CONFIG_INITIALIZED_VALUE   0x00

/** Adresa v EEPROM, kam sa ulozi priznak inicializacie hodnoty pre mod zobrazenia otacok. */
#define CONFIG_ADDR_DISP_TYPE_INIT   0x00

/** Adresa v EEPROM, kam sa ulozi nastaveny mod zobrazenia otacok. */
#define CONFIG_ADDR_DISP_TYPE_VALUE  0x01

/** Adresa v EEPROM, kam sa ulozi priznak inicializacie sirky pulzu ventilatora na max. otackach. */
#define CONFIG_ADDR_FAN_MAX_INIT     0x10

/** Adresa v EEPROM, kam sa ulozi sirka pulzu ventilatora na max. otackach. */
#define CONFIG_ADDR_FAN_MAX_VALUE    0x11

/** Adresa v EEPROM, kam sa ulozi priznak inicializacie sirky pulzu ventilatora na min. otackach. */
#define CONFIG_ADDR_FAN_MIN_INIT     0x20

/** Adresa v EEPROM, kam sa ulozi sirka pulzu ventilatora na min. otackach. */
#define CONFIG_ADDR_FAN_MIN_VALUE    0x21

/** Predvoleny mod zobrazenia otacok, ak este nebol ziadny zvoleny. */
#define DISPLAY_TYPE_DEFAULT  DISPLAY_TYPE_SET

static bool config_is_initialized(uint16_t address);
static void config_save(uint16_t address_init, uint16_t address_data, const uint8_t * const data, uint16_t data_size);

/**
 * @brief Zistenie, ci je hodnota v EEPROM inicializovana.
 *
 * @param address Adresa bajtu v EEPROM.
 * @retval true Hodnota je inicializovana.
 * @retval false Hodnota nie je inicializovana.
 */
bool
config_is_initialized(uint16_t address)
{
	uint8_t b;
	
	eeprom_read(address, &b, sizeof(b));	
	
	return (b != CONFIG_UNINITIALIZED_VALUE);
}

/**
 * @brief Ulozenie dat do EEPROM a nastavenie priznakoveho bajtu inicializacie dat.
 *
 * @param address_init Adresa priznakoveho bajtu inicializacie dat.
 * @param address_data Adresa kam budu zapisane data.
 * @param data Ukazovatel na buffer dat pre zapis.
 * @param data_size Pocet bajtov pre zapis.
 */
void
config_save(uint16_t address_init, uint16_t address_data, const uint8_t * const data, uint16_t data_size)
{
	// Zapise sa hodnota
	eeprom_write(address_data, data, data_size);
	// Zapise sa priznak inicializacie hodnoty
	uint8_t i = CONFIG_INITIALIZED_VALUE;
	eeprom_write(address_init, &i, sizeof(i));	
}

/**
 * @brief Zistenie, ci je type zobrazenia ulozeny v EEPROM inicializovany.
 *
 * @retval true Typ je inicializovany.
 * @retval false Typ nie je inicializovany.
 */
bool
config_disp_type_is_initialized(void)
{
	return config_is_initialized(CONFIG_ADDR_DISP_TYPE_INIT);
}

/**
 * @brief Ulozenie typu zobrazenie do EEPROM.
 *
 * @param disp_type Typ zobrazenia.
 */
void
config_disp_type_save(uint8_t disp_type)
{
	config_save(CONFIG_ADDR_DISP_TYPE_INIT, CONFIG_ADDR_DISP_TYPE_VALUE, &disp_type, sizeof(disp_type));
}

/**
 * @brief Nacitanie typu zobrazenia z EEPROM.
 *
 * Ak este nebolo ziadne zobrazenie ulozene, ulozi sa predvolena hodnota a ta sa aj vrati volajucemu.
 *
 * @return Typ zobrazenia.
 */
uint8_t
config_disp_type_load(void)
{
	uint8_t display_type = DISPLAY_TYPE_NONE;
		
	if (config_disp_type_is_initialized()) {
		eeprom_read(CONFIG_ADDR_DISP_TYPE_VALUE, &display_type, sizeof(display_type));
	}
	
	if (display_type >= DISPLAY_TYPE_COUNT) {
		display_type = DISPLAY_TYPE_DEFAULT;
		config_disp_type_save(display_type);
	}
	
	return display_type;
}

/**
 * @brief Zisti, ci je hodnota sirky pulzu na max. otackach inicializovana v EEPROM.
 *
 * @retval true Hodnota je inicializovana.
 * @retval false Hodnota nie je inicializovana. 
 */
bool
config_is_fan_max_rpm_pulse_time_init(void)
{
	return config_is_initialized(CONFIG_ADDR_FAN_MAX_INIT);	
}

/**
 * @brief Ulozenie sirky pulzu na max. otackach do EEPROM.
 *
 * @param pulse_time Sirka pulzu.
 */
void
config_fan_max_rpm_pulse_time_save(uint16_t pulse_time)
{
	config_save(CONFIG_ADDR_FAN_MAX_INIT, CONFIG_ADDR_FAN_MAX_VALUE, (uint8_t *) &pulse_time, sizeof(pulse_time));
}

/**
 * @brief Nacitanie sirky pulzu na max. otackach z EEPROM.
 *
 * @return Sirka pulzu.
 */
uint16_t
config_fan_max_rpm_pulse_time_load(void)
{
	uint16_t w;
	eeprom_read(CONFIG_ADDR_FAN_MAX_VALUE, (uint8_t *) &w, sizeof(w));
	return w;
}

/**
 * @brief Zisti, ci je hodnota sirky pulzu na min. otackach inicializovana v EEPROM.
 *
 * @retval true Hodnota je inicializovana.
 * @retval false Hodnota nie je inicializovana. 
 */
bool
config_is_fan_min_rpm_pulse_time_init(void)
{
	return config_is_initialized(CONFIG_ADDR_FAN_MIN_INIT);
}

/**
 * @brief Ulozenie sirky pulzu na min. otackach do EEPROM.
 *
 * @param pulse_time Sirka pulzu.
 */
void
config_fan_min_rpm_pulse_time_save(uint16_t pulse_time)
{
	config_save(CONFIG_ADDR_FAN_MIN_INIT, CONFIG_ADDR_FAN_MIN_VALUE, (uint8_t *) &pulse_time, sizeof(pulse_time));
}

/**
 * @brief Nacitanie sirky pulzu na min. otackach z EEPROM.
 *
 * @return Sirka pulzu.
 */
uint16_t
config_fan_min_rpm_pulse_time_load(void)
{
	uint16_t w;
	eeprom_read(CONFIG_ADDR_FAN_MIN_VALUE, (uint8_t *) &w, sizeof(w));
	return w;
}

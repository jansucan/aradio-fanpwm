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

#include <avr/io.h>

#include <fan_controller/hw/eeprom.h>
#include <fan_controller/error.h>

/** Velkost pamate EEPROM v bajtoch */
#define EEPROM_SIZE_IN_BYTES  512U // bajtov

static void eeprom_write_byte(uint16_t address, uint8_t byte);
static void eeprom_read_byte(uint16_t address, uint8_t *const byte);
static void eeprom_wait_until_ready(void);
static void eeprom_check_address_and_size(uint16_t address, uint16_t op_size);

/**
 * @brief Zapis dat z EEPROM.
 *
 * @param address Adresa kam sa bude zapisovat.
 * @param data Ukazovatel na buffer, odkial sa budu brat bajty pre zapis do EEPROM.
 * @param data_size Pocet bajtov co sa budu zapisovat.
 */
void
eeprom_write(uint16_t address, const uint8_t * const data, uint16_t data_size)
{
	eeprom_check_address_and_size(address, data_size);
		
	for (uint16_t i = 0; i < data_size; i++) {
		eeprom_write_byte(address, data[i]);
		++address;	
	}
}

/**
 * @brief Precitanie dat z EEPROM.
 *
 * @param address Adresa odkial sa bude citat.
 * @param data Ukazovatel na buffer, kde sa precitane bajty ulozia.
 * @param data_size Pocet bajtov co sa budu citat.
 */
void
eeprom_read(uint16_t address, uint8_t * const data, uint16_t data_size)
{
	eeprom_check_address_and_size(address, data_size);
	
	for (uint16_t i = 0; i < data_size; i++) {
		eeprom_read_byte(address, &(data[i]));
		++address;
	}
}

/**
 * @brief Zapis bajtu z EEPROM.
 *
 * @param address Adresa kam sa bude zapisovat.
 * @param byte Bajt pre zapis na adresu @p address.
 */
void
eeprom_write_byte(uint16_t address, uint8_t byte)
{
	eeprom_wait_until_ready();
	EEAR = address;
	EEDR = byte;
	EECR |= (1 << EEMWE);
	EECR |= (1 << EEWE);
}

/**
 * @brief Precitanie bajtu z EEPROM.
 *
 * @param address Adresa odkial sa bude citat.
 * @param byte Ukazovatel na bajt, kde bude ulozena precitana hodnota.
 */
void
eeprom_read_byte(uint16_t address, uint8_t *const byte)
{
	eeprom_wait_until_ready();
	EEAR = address;
	EECR |= (1<<EERE);
	*byte = EEDR;
}

/**
 * @brief Pocka, az bude pamat EEPROM pripravena na dalsiu operaciu.
 *
 * Blokujuca funkcia.
 */
void
eeprom_wait_until_ready(void)
{
	while ((EECR & (1 << EEWE)) != 0) {
		;
	}
}

/**
 * @brief Skontroluje ci sa velkost dat @p op_size vojde do EEPROM od adresy @p address.
 *
 * Ak sa data nezmestia, vyvola chybu.
 *
 * @param address Adresa v EEPROM.
 * @param op_size Pocet bajtov dat.
 */
void
eeprom_check_address_and_size(uint16_t address, uint16_t op_size)
{
	if ((address >= EEPROM_SIZE_IN_BYTES) || ((address + op_size) > EEPROM_SIZE_IN_BYTES)) {
		error();
	}	
}
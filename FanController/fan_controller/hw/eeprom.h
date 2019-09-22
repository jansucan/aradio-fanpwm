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

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>

void eeprom_write(uint16_t address, const uint8_t * const data, uint16_t data_size);
void eeprom_read(uint16_t address, uint8_t * const data, uint16_t data_size);

#endif /* EEPROM_H_ */
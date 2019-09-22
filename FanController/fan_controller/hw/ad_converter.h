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

#ifndef AD_CONVERTER_H_
#define AD_CONVERTER_H_

#include <stdint.h>

void adc_init(void);
uint8_t adc_get_converted_value(void);
void adc_override_value_set(uint8_t val);
void adc_override_value_delete(void);

#endif /* AD_CONVERTER_H_ */
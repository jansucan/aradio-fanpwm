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

#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>
#include <stdbool.h>

void     delay_init(void);
void     delay_ms(uint16_t ms);
uint32_t delay_get_deadline_ms(uint16_t ms);
bool     delay_has_deadline_expired(uint32_t deadline);

#endif /* DELAY_H_ */
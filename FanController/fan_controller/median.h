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

#ifndef MEDIAN_H_
#define MEDIAN_H_

#include <stdint.h>

/** Minimalny pocet vzoriek pre vypocet medianu. */
#define MEDIAN_MIN_SAMPLES 3U

/** Maximalny pocet vzoriek pre vypocet medianu. */
#define MEDIAN_MAX_SAMPLES 16U

/** Vzorka pre median. */
typedef struct {
	uint16_t value; /**< Hodnota vzorky. */
	uint8_t age_index; /**< Index do FIFO age v bufferi medianu.  */
} median_sample_t;

/** Buffer pre vypocet medianu. */
typedef struct {
	uint8_t number_of_elements; /**< Pocet aktualne ulozenych vzoriek. */
	uint8_t capacity; /**< Maximalny pocet ulozenych vzoriek. */
	median_sample_t samples[MEDIAN_MAX_SAMPLES]; /**< Vzorky z ktorych sa pocita median. */
	uint8_t age[MEDIAN_MAX_SAMPLES]; /**< FIFO. Na indexe 0 je index najstarsej vzorky v poli samples, na indexe 1 je index druhej najstarsej, atd. Pouziva sa pre rychle najdenie najstarsieho prvku. */
} median_buffer_t;

void median_init_buffer(median_buffer_t * const buf, uint8_t capacity);
void median_save_sample(median_buffer_t * const buf, uint16_t sample);
uint16_t median_get(const median_buffer_t * const buf);

#endif /* MEDIAN_H_ */
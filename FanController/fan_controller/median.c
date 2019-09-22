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

#include <string.h>

#include <fan_controller/median.h>
#include <fan_controller/error.h>

static void median_delete_oldest_sample(median_buffer_t * const buf);

/**
 * @brief Vymazanie najstarsej vzorky z bufferu medianu.
 *
 * @param buf Ukazovatel na buffer medianu.
 */
void
median_delete_oldest_sample(median_buffer_t * const buf)
{	
	// Ziskat index najstarsieho prvku v poli buf->samples
	const uint8_t oldest_index = buf->age[0];
	
	if (oldest_index >= buf->number_of_elements) {
		error();
	}
	// Pocet prvkov, ktore sa budu presuvat na prazdne miesto po zmazanom prvku
	const uint8_t samples_to_move = buf->number_of_elements - oldest_index - 1;
	// Ak je co presuvat, presunie sa
	if (samples_to_move > 0) {
		memmove(buf->samples + oldest_index,
				buf->samples + oldest_index + 1,
				samples_to_move * sizeof(buf->samples[0]));
	}
	
	// Zmensi sa pocet ulozenych prvkov
	--(buf->number_of_elements);
	
	// Aktualizovat pozicie presunutych prvkov vo FIFO bufferi
	for (uint8_t i = oldest_index; i < (oldest_index + samples_to_move); ++i) {
		// Prvky sa posunuli k nizsim indexom
		--(buf->age[(buf->samples[i]).age_index]);
	}
	
	// Treba aktualizovat FIFO indexov do buf->samples. Vyhodi sa najstarsi prvok
	memmove(buf->age,
			buf->age + 1,
			buf->number_of_elements * sizeof(buf->samples[0]));
			
	// Aktualizovat pozicie presunutych prvkov v bufferi vzoriek
	for (uint8_t i = 0; i < buf->number_of_elements; ++i) {
		// Prvky sa posunuli k nizsim indexom
		--(buf->samples[buf->age[i]].age_index);
	}
}

/**
 * @brief Inicializacia bufferu medianu.
 *
 * Buffer medianu sa musi vzdy inicializovat pred prvym pouzitim.
 *
 * @param buf Ukazovatel na buffer medianu.
 * @param capacity Kapacita bufferu, tj. max. pocet vzoriek z ktorych sa bude pocitat median.
 */
void
median_init_buffer(median_buffer_t * const buf, uint8_t capacity)
{
	if ((capacity < MEDIAN_MIN_SAMPLES) ||(capacity > MEDIAN_MAX_SAMPLES)) {
		error();
	}
	buf->number_of_elements = 0;
	buf->capacity = capacity;
}

/**
 * @brief Vlozenie prvku do bufferu pre median.
 *
 * @param buf Ukazovatel na buffer medianu.
 * @param sample Hodnota pre ulozenie do bufferu medianu.
 */
void
median_save_sample(median_buffer_t * const buf, uint16_t sample)
{
	if (buf->number_of_elements >= buf->capacity) {
		// Buffer je plny, vymaze sa najtarsi prvok
		median_delete_oldest_sample(buf);
	}
	
	// Buffer nie je plny, vzorka sa ulozi na koniec bufferu
	buf->samples[buf->number_of_elements].value = sample;
	buf->samples[buf->number_of_elements].age_index = buf->number_of_elements;
	buf->age[buf->number_of_elements] = buf->number_of_elements;
	
	// Vzorka sa zaradi na spravne miesto
	for (uint8_t j = buf->number_of_elements, i = j - 1; j > 0; --i, --j) {
		if (buf->samples[j].value < buf->samples[i].value) {
			// Aktualizovat casove znacky
			// Vzorka na pozicii i pojde na vyssi index
			++(buf->age[buf->samples[i].age_index]);
			// Vzorka na pozicii j pojde na nizsi index
			--(buf->age[buf->samples[j].age_index]);
			// Vymenit vzorky
			median_sample_t t;
			t = buf->samples[j];
			buf->samples[j] = buf->samples[i];
			buf->samples[i] = t;
		}
	}
	
	// Aktualizovat pocet ulozenych prvkov
	++(buf->number_of_elements);
}

/**
 * @brief Ziskanie medianu z bufferu.
 *
 * @param buf Ukazovatel na buffer medianu.
 * @return Median z doteraz ulozenych hodnot.
 */
uint16_t
median_get(const median_buffer_t * const buf)
{
	return (buf->samples[buf->number_of_elements / 2]).value;
}
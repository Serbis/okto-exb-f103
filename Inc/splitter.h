#ifndef SPLITTER_H_
#define SPLITTER_H_

#include <stdint.h>
#include <stdbool.h>

/** Compare was success */
#define CMPR_SUCCESS 0

/** Bad regexp pattern */
#define CMPR_ERR_REGEXP 1

/** Element does not matched */
#define CMPR_ERR_MATCH 2

/** Splitter elements count < minc */
#define CMPR_ERR_SPLLEN 3

/** Result of the spritString funstion */
typedef struct StringSplitter {

	/** Array of elements */
	char **elems;

	/** Count of elements */
	uint32_t size;
} StringSplitter;

/** String splitter comarasion result */
typedef struct StringCmpResult {

	/** Result type, see CMPR defines*/
	uint8_t result;

	/** Element where error was occurred */
	uint8_t perr;

	/** Compare difference. For example if splitter has 3 elements and it will be compared with
	 *  3 elements, 2 will be set here */
	uint8_t diff;
} StringCmpResult;

StringSplitter* Splitter_splitString(char *str, char *splitter, uint32_t maxels, uint32_t maxelc);
StringCmpResult* Splitter_compareString(StringSplitter *splitter, uint8_t minc, uint8_t cmplen, char *cmp, ...);
char* Splitter_getStringElem(StringSplitter *splitter, uint32_t pos);

#endif

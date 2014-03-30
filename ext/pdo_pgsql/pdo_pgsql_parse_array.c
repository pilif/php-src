#include "Zend/zend.h"
#include "Zend/zend_API.h"
#include "pdo_pgsql_parse_array.h"

#define MAX_DIMS 16

static int array_cleanup(char **str, int *len){
	int i, depth = 1;

	if ((*str)[0] != '[') return -1;

	for (i=1 ; depth > 0 && i < *len ; i++) {
		if ((*str)[i] == '[')
			depth += 1;
		else if ((*str)[i] == ']')
			depth -= 1;
	}
	if ((*str)[i] != '=') return -1;

	*str = &((*str)[i+1]);
	*len = *len - i - 1;
	return 0;
}

#define SCAN_ERR -1
#define SCAN_EOF    0
#define SCAN_BEGIN  1
#define SCAN_END    2
#define SCAN_TOKEN  3
#define SCAN_QUOTED 4

static int array_tokenize(const char *str, int strlength, int *pos, char** token, int *length, int *quotes){
	int i, l;
	int q, b, res;

	if (*pos == strlength) {
		return SCAN_EOF;
	}
	else if (str[*pos] == '{') {
		*pos += 1;
		return SCAN_BEGIN;
	}
	else if (str[*pos] == '}') {
		*pos += 1;
		if (str[*pos] == ',')
			*pos += 1;
		return SCAN_END;
	}

	/* now we start looking for the first unquoted ',' or '}', the only two
	   tokens that can limit an array element */
	q = 0; /* if q is odd we're inside quotes */
	b = 0; /* if b is 1 we just encountered a backslash */
	res = SCAN_TOKEN;

	for (i = *pos ; i < strlength ; i++) {
		switch (str[i]) {
		case '"':
			if (b == 0)
				q += 1;
			else
				b = 0;
			break;

		case '\\':
			res = SCAN_QUOTED;
			if (b == 0)
				b = 1;
			else
				/* we're backslashing a backslash */
				b = 0;
			break;

		case '}':
		case ',':
			if (b == 0 && ((q&1) == 0))
				goto tokenize;
			break;

		default:
			/* reset the backslash counter */
			b = 0;
			break;
		}
	}

 tokenize:
	/* remove initial quoting character and calculate raw length */
	*quotes = 0;
	l = i - *pos;
	if (str[*pos] == '"') {
		*pos += 1;
		l -= 2;
		*quotes = 1;
	}

	if (res == SCAN_QUOTED) {
		const char *j, *jj;
		char *buffer = (char*)emalloc(l+1);
		if (buffer == NULL) {
			return SCAN_ERR;
		}

		*token = buffer;

		for (j = str + *pos, jj = j + l; j < jj; ++j) {
			if (*j == '\\') { ++j; }
			*(buffer++) = *j;
		}

		*buffer = '\0';
		*length = (int) (buffer - *token);
	}
	else {
		*token = (char *)&str[*pos];
		*length = l;
	}

	*pos = i;

	/* skip the comma and set position to the start of next token */
	if (str[i] == ',') *pos += 1;

	return res;
}

static int array_scan(const char *str, int strlength, zval *arr, pg_array_adder adder, char** error){
	int state, quotes = 0;
	int length = 0, pos = 0;
	char *token;


	/* TODO: stack of max dimensions */
	zval* array_stack[MAX_DIMS];
	size_t stack_index = 0;


	while (1) {
		token = NULL;
		state = array_tokenize(str, strlength,
									&pos, &token, &length, &quotes);

		if (state == SCAN_TOKEN || state == SCAN_QUOTED) {
			if (!quotes && length == 4 && (strncasecmp(token, "null", 4) == 0)){
				add_next_index_null(arr);
			} else {
				if (adder(arr, token, length) < 0){
					*error = "failed to populate array";
					return -1;
				}
			}

			if (state == SCAN_QUOTED) efree(token);
		}else if (state == SCAN_BEGIN) {
			if (stack_index == MAX_DIMS) {
				*error = "array too deeply nested";
				return -1;
			}
			zval* sub_array;
			MAKE_STD_ZVAL(sub_array);
			array_init(sub_array);
			add_next_index_zval(arr, sub_array);

			array_stack[stack_index++] = arr;
			arr = sub_array;
		}else if (state == SCAN_ERR) {
			return -1;
		}else if (state == SCAN_END) {
			if (stack_index == 0) {
				*error = "unbalanced curlies";
				return -1;
			}
			arr = array_stack[stack_index--];
		}
		else if (state ==  SCAN_EOF)
			break;
	}

	return 0;
}

int add_string_to_array(zval *array, char *str, int len){
	if (add_next_index_stringl(array, str, len, 1) == FAILURE)
		return -1;

	return 0;
}

int add_long_to_array(zval *array, char *str, int len){
	long value = strtol(str, NULL, 10);

	if ( (value == 0) && (errno == ERANGE) ){
		return add_next_index_stringl(array, str, len, 1);
	}

	if (add_next_index_long(array, value) == FAILURE)
		return -1;

	return 0;
}

int add_double_to_array(zval *array, char *str, int len){
	if (add_next_index_double(array, strtod(str, NULL)) == FAILURE)
		return -1;
	return 0;
}

int add_bool_to_array(zval *array, char *str, int len){
	if (! ((str[0] == 't') || (str[0] == 'f'))) return -1;

	if (add_next_index_bool(array, str[0] == 't') == FAILURE)
		return -1;

	return 0;
}

zval* pdo_pgsql_parse_array(char *str, int len, pg_array_adder adder, char **error){
	zval *arr;
	MAKE_STD_ZVAL(arr);

	if (str == NULL) {
		ZVAL_NULL(arr);
		return arr;
	}

	if (str[0] == '[')
		array_cleanup(&str, &len);

	if (str[0] != '{') {
		*error = "array does not start with '{'\n";
		ZVAL_NULL(arr);
		return arr;
	}
	if (str[1] == '\0') {
		*error = "invalid array: {\n";
		ZVAL_NULL(arr);
		return arr;
	}

	array_init(arr);

	if (array_scan(&str[1], len-2, arr, adder, error) < 0) {
		*error = "parse error\n";
		ZVAL_NULL(arr);
		return arr;
	}

	return arr;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

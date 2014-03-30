#ifndef PDO_PGSQL_PARSE_ARRAY_H
#define PDO_PGSQL_PARSE_ARRAY_H

typedef int (* pg_array_adder)(zval*, char*, int);

zval* pdo_pgsql_parse_array(char *str, int len, pg_array_adder adder, char **error);

int add_string_to_array(zval *array, char *str, int len);
int add_long_to_array(zval *array, char *str, int len);
int add_double_to_array(zval *array, char *str, int len);
int add_bool_to_array(zval *array, char *str, int len);

#endif  /* PDO_PGSQL_PARSE_ARRAY_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

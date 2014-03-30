--TEST--
PDO PgSQL identifier quoting
--SKIPIF--
<?php # vim:se ft=php:
if (!extension_loaded('pdo') || !extension_loaded('pdo_pgsql')) die('skip not loaded');
require dirname(__FILE__) . '/config.inc';
require dirname(__FILE__) . '/pgsql_pdo_test.inc';
PgsqlPDOTest::skip();
?>
--FILE--
<?php
require dirname(__FILE__) . '/pgsql_pdo_test.inc';
$pdo = PgsqlPDOTest::test_factory(dirname(__FILE__) . '/common.phpt');
var_dump($pdo->pgsqlQuoteIdentifier('foo'));
var_dump($pdo->pgsqlQuoteIdentifier('fo"o'));
?>
--EXPECT--
string(5) ""foo""
string(7) ""fo""o""

--TEST--
PDO PgSQL JSON parsing
--SKIPIF--
<?php # vim:se ft=php:
if (!extension_loaded('pdo') || !extension_loaded('pdo_pgsql')) die('skip not loaded');
if (!extension_loaded('json')) die('skip json extension not loaded');
require dirname(__FILE__) . '/config.inc';
require dirname(__FILE__) . '/pgsql_pdo_test.inc';
PgsqlPDOTest::skip(['min_server_version' => '9.2']);
?>
--FILE--
<?php
require dirname(__FILE__) . '/pgsql_pdo_test.inc';
$pdo = PgsqlPDOTest::test_factory(dirname(__FILE__) . '/common.phpt');
$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
$pdo->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);


var_dump($pdo->getAttribute(PDO::PGSQL_ATTR_ADVANCED_TYPE_CONVERSIONS));
var_dump($pdo->setAttribute(PDO::PGSQL_ATTR_ADVANCED_TYPE_CONVERSIONS, true));
$st = $pdo->prepare('select \'{"foo": "bar", "baz": {"foo": 1}}\'::json');
$st->execute();
var_dump($st->fetchAll());

$pdo->setAttribute(PDO::PGSQL_ATTR_ADVANCED_TYPE_CONVERSIONS, false);
$st = $pdo->prepare('select \'{"foo": "bar", "baz": {"foo": 1}}\'::json');
$st->execute();
var_dump($st->fetchAll());

$st = $pdo->prepare('select \'{"foo": "bar", "baz": {"foo": 1}}\'::json', [PDO::PGSQL_ATTR_ADVANCED_TYPE_CONVERSIONS => true]);
$st->execute();
var_dump($st->fetchAll());

$pdo->setAttribute(PDO::PGSQL_ATTR_ADVANCED_TYPE_CONVERSIONS, true);
$st = $pdo->prepare('select \'{"foo": "bar", "baz": {"foo": 1}}\'::json', [PDO::PGSQL_ATTR_ADVANCED_TYPE_CONVERSIONS => false]);
$st->execute();
var_dump($st->fetchAll());

?>
--EXPECT--
bool(false)
bool(true)
array(1) {
  [0]=>
  array(1) {
    ["json"]=>
    array(2) {
      ["foo"]=>
      string(3) "bar"
      ["baz"]=>
      array(1) {
        ["foo"]=>
        int(1)
      }
    }
  }
}
array(1) {
  [0]=>
  array(1) {
    ["json"]=>
    string(33) "{"foo": "bar", "baz": {"foo": 1}}"
  }
}
array(1) {
  [0]=>
  array(1) {
    ["json"]=>
    array(2) {
      ["foo"]=>
      string(3) "bar"
      ["baz"]=>
      array(1) {
        ["foo"]=>
        int(1)
      }
    }
  }
}
array(1) {
  [0]=>
  array(1) {
    ["json"]=>
    string(33) "{"foo": "bar", "baz": {"foo": 1}}"
  }
}
<?php # vim:se ft=php:
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';

class PgsqlPDOTest extends PDOTest {
    static function skip($conditions = null){
        if ($conditions === null) $conditions = [];
        if (!$conditions) return parent::skip();

        try {
            $db = PDOTest::factory();
            if ($conditions['min_server_version']){
                if (version_compare($db->getAttribute(PDO::ATTR_SERVER_VERSION), $conditions['min_server_version']) == -1){
                    die("skip server too old for this test");
                }
            }
        } catch (PDOException $e) {
            die("skip " . $e->getMessage());
        }

    }
}

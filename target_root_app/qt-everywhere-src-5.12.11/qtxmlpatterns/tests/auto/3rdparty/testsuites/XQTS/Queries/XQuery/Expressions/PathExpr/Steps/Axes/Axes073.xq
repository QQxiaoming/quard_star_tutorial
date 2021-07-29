(: Name: Axes073 :)
(: Description: Test '//self::node()' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//self::node())

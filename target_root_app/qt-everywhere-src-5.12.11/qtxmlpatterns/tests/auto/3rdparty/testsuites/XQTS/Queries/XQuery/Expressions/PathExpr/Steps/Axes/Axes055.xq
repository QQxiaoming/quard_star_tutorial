(: Name: Axes055 :)
(: Description: Test '/self::node()' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/self::node())

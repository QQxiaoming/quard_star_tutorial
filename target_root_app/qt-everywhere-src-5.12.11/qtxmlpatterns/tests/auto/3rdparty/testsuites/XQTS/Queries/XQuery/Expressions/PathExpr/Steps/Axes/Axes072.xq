(: Name: Axes072 :)
(: Description: Test '//self::*' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//self::*)

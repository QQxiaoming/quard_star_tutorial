(: Name: Axes061 :)
(: Description: Test '/descendant-or-self::node()' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/descendant-or-self::node())

(: Name: Axes059 :)
(: Description: Test '/descendant-or-self::*' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/descendant-or-self::*)

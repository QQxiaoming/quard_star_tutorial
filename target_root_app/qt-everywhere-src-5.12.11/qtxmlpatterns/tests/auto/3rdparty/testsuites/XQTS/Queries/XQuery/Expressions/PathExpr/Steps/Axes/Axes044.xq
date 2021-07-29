(: Name: Axes044 :)
(: Description: Test '/child::*' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/child::*)

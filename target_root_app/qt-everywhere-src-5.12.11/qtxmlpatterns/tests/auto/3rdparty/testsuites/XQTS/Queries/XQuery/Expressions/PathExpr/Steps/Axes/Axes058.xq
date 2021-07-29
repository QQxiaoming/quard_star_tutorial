(: Name: Axes058 :)
(: Description: Test '/descendant::node()' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/descendant::node())

(: Name: Axes047 :)
(: Description: Test implied child axis in '/*' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/*)

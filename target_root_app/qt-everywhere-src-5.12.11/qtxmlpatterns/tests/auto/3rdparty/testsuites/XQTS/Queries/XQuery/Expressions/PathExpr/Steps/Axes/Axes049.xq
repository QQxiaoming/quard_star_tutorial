(: Name: Axes049 :)
(: Description: Test implied child axis in '/node()' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/node())

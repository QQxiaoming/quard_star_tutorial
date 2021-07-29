(: Name: Axes078 :)
(: Description: Test implied child axis in element//element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center//south)

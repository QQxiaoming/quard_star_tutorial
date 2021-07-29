(: Name: Axes004 :)
(: Description: Test implied child axis in '*' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/*)

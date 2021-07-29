(: Name: Axes005 :)
(: Description: Test implied child axis with element name, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/south-east)

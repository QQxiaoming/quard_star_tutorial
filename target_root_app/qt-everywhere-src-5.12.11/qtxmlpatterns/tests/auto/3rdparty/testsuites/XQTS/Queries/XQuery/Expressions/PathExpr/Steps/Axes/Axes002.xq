(: Name: Axes002 :)
(: Description: Path 'child::' with element name from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/child::south-east)

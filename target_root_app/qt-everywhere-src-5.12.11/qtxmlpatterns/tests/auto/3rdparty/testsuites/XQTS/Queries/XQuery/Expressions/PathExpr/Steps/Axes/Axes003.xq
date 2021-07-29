(: Name: Axes003 :)
(: Description: Path 'child::node()' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/child::node())

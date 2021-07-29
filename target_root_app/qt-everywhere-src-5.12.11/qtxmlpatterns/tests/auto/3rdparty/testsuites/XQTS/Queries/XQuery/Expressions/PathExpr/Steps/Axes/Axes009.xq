(: Name: Axes009 :)
(: Description: Path 'attribute::node()' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west/attribute::node())

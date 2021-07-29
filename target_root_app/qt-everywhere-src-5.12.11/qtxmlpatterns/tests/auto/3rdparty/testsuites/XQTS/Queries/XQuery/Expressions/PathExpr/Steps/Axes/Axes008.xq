(: Name: Axes008 :)
(: Description: Path 'attribute::' with name of attribute, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west/attribute::west-attr-2)

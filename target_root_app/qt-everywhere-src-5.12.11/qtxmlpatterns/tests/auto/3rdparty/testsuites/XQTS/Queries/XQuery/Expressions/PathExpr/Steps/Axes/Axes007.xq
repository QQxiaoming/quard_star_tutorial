(: Name: Axes007 :)
(: Description: Path 'attribute::*' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west/attribute::*)

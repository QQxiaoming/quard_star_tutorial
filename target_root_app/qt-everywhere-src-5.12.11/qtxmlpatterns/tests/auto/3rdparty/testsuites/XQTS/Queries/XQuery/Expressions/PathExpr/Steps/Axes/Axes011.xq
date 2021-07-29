(: Name: Axes011 :)
(: Description: Path '@' with name of attribute, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west/@west-attr-2)

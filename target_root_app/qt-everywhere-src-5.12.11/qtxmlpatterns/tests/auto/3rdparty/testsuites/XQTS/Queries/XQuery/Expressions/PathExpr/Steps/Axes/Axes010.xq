(: Name: Axes010 :)
(: Description: Path '@*' (abbreviated syntax) from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west/@*)

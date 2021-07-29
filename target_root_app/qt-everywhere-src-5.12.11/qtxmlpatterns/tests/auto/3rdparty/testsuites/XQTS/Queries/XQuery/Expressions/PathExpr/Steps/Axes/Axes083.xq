(: Name: Axes083 :)
(: Description: Path '//@*' (abbreviated syntax) after an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west//@*)

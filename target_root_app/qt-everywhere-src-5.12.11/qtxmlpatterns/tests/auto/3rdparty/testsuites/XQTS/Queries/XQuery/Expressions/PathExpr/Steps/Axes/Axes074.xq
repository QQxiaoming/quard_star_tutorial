(: Name: Axes074 :)
(: Description: Path '//child::*' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center//child::*)

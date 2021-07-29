(: Name: Axes080 :)
(: Description: Path '//attribute::*' after an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west//attribute::*)

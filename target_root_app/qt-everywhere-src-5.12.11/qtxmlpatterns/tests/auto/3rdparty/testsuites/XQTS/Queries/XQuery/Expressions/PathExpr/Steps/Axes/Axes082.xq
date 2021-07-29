(: Name: Axes082 :)
(: Description: Path '//attribute::node()' after an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//west//attribute::node())

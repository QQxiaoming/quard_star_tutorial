(: Name: Axes033 :)
(: Description: Path 'descendant::node()' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/descendant::node())

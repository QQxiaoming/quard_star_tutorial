(: Name: Axes017 :)
(: Description: Path 'parent::node()' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/parent::node())

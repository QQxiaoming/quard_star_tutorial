(: Name: Axes013 :)
(: Description: Path 'parent::*' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/parent::*)

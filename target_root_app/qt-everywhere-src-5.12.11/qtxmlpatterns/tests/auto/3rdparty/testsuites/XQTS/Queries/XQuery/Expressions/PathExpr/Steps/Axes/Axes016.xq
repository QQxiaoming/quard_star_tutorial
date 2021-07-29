(: Name: Axes016 :)
(: Description: Path 'parent::' with specified element name that is not found, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/parent::nowhere)

(: Name: Axes015 :)
(: Description: Path 'parent::' with specified element name, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/parent::near-north)

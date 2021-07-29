(: Name: Axes032 :)
(: Description: Path 'descendant::' with specified element name, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/descendant::south)

(: Name: Axes035 :)
(: Description: Path 'descendant-or-self::' with specified element name, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/descendant-or-self::south)

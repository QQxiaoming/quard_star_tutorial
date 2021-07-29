(: Name: Axes036 :)
(: Description: Path 'descendant-or-self::' with name of self specified. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/descendant-or-self::center)

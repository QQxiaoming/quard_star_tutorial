(: Name: Axes041 :)
(: Description: Path 'descendant-or-self::node()' from an attribute. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-3/descendant-or-self::node())

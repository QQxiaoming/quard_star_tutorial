(: Name: Axes037 :)
(: Description: Path 'descendant-or-self::node()' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/descendant-or-self::node())

(: Name: Axes030 :)
(: Description: Path 'self::node()' from a text node. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/text()/self::node())

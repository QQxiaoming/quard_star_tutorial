(: Name: Axes023 :)
(: Description: Path 'self::node()' from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/self::node())

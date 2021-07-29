(: Name: Axes021 :)
(: Description: Path 'self::' with specified element name, from an element. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/self::center)

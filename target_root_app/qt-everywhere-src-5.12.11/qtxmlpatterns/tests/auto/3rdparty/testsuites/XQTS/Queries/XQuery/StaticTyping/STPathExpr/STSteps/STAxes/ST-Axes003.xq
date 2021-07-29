(: Name: ST-Axes003 :)
(: Description: Path 'self::' with specified attribute name, from that attribute. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-3/self::center-attr-3)

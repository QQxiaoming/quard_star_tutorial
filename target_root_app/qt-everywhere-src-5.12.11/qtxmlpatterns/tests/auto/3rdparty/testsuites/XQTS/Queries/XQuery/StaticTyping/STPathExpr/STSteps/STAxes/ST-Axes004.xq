(: Name: ST-Axes004 :)
(: Description: Path 'self::' with specified attribute name that is not found, from an attribute. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-1/self::nowhere)

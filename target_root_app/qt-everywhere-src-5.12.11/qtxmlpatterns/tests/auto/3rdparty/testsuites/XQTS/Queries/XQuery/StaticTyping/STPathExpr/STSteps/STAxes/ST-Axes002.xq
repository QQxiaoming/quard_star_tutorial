(: Name: ST-Axes002 :)
(: Description: Path 'self::*' from an attribute. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-2/self::*)

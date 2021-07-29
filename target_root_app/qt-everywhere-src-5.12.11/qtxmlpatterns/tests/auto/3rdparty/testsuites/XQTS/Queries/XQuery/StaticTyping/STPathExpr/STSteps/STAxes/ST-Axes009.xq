(: Name: ST-Axes009 :)
(: Description: descendant-or-self::name from an attribute gets nothing, even with attribute's name :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-2/descendant-or-self::center-attr-2)

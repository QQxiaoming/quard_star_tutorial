(: Name: ST-Axes008 :)
(: Description: descendant-or-self::name from an attribute gets nothing :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-2/descendant-or-self::far-south)

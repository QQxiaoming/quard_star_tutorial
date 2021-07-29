(: Name: ST-Axes007 :)
(: Description: descendant-or-self::* from an attribute gets nothing :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/@center-attr-1/descendant-or-self::*)

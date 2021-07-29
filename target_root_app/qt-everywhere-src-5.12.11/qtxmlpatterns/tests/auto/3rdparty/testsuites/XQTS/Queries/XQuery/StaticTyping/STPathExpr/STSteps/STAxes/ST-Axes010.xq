(: Name: ST-Axes010 :)
(: Description: descendant-or-self::* from a text node gets nothing :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//center/text()/descendant-or-self::*)

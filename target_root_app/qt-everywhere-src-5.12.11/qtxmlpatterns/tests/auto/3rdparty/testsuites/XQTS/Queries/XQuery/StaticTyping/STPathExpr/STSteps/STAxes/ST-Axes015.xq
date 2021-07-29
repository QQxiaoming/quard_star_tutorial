(: Name: ST-Axes015 :)
(: Description: /self::* gets nothing because root is not an element node :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/self::*)

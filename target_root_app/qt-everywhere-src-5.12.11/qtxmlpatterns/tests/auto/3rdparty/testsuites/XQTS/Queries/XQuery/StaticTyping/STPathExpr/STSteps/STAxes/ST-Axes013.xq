(: Name: ST-Axes013 :)
(: Description: /parent::* gets nothing because root can't have parent :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/parent::*)

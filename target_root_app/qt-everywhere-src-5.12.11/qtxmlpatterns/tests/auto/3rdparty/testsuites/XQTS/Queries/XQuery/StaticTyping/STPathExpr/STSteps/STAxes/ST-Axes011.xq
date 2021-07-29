(: Name: ST-Axes011 :)
(: Description: /attribute::* gets nothing because root can't have attributes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/attribute::*)

(: Name: ST-Axes014 :)
(: Description: /.. gets nothing because root can't have parent :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/..)

(: Name: Axes012 :)
(: Description: Test '/' alone. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count( $input-context/ )

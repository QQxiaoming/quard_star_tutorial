(: Name: Axes068 :)
(: Description: Test '//attribute::*' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context//attribute::*)

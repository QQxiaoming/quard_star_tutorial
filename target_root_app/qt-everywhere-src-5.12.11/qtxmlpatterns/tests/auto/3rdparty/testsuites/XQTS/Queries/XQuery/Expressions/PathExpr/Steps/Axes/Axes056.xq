(: Name: Axes056 :)
(: Description: Test '/descendant::*' absolute path. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count($input-context/descendant::*)

(: Name: fn-doc-15 :)
(: Description: Evaluation of fn:doc as an argument to the fn:nilled function.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:nilled(fn:doc($input-context)))
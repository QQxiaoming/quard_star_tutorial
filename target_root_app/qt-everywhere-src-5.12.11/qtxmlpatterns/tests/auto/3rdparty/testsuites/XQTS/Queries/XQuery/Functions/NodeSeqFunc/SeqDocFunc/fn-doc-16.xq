(: Name: fn-doc-16 :)
(: Description: Evaluation of fn:doc as an argument to the fn:node-name function.:)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

fn:count(fn:node-name(fn:doc($input-context)))
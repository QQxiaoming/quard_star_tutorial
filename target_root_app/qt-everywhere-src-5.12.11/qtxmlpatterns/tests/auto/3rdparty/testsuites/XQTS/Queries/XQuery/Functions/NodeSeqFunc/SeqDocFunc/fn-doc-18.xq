(: Name: fn-doc-18 :)
(: Description: Evaluation of fn:doc used with "is" operator and the fn:not function.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)
fn:not(fn:doc($input-context) is fn:doc($input-context))
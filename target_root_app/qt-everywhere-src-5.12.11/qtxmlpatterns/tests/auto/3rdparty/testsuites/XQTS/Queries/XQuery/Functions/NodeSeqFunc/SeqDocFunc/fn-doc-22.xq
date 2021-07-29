(: Name: fn-doc-22 :)
(: Description: Evaluation of fn:doc used with "is" operator and the fn:false function with "or" operator.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)
(fn:doc($input-context) is fn:doc($input-context)) or fn:false()
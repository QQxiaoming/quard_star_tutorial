(: Name: fn-doc-21 :)
(: Description: Evaluation of fn:doc used with "is" operator and the fn:true function with "or" operator.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)
(fn:doc($input-context) is fn:doc($input-context)) or fn:true()
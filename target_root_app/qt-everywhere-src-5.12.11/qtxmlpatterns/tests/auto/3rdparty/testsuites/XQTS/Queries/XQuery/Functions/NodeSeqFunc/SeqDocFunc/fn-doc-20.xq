(: Name: fn-doc-20 :)
(: Description: Evaluation of fn:doc used with "is" operator and the fn:false function with "and" operator.:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

(fn:doc($input-context) is fn:doc($input-context)) and fn:false()
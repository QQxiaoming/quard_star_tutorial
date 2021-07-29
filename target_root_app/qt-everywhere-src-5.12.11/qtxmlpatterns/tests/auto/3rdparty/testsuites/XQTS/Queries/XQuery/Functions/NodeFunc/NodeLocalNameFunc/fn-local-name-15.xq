(: Name: fn-local-name-15:)
(: Description: Evaluation of the fn:local-name function argument set to a directly constructed PI node.:)
(: Use fn:string.  :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:local-name(<?format role="output" ?>))

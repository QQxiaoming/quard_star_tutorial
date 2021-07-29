(: Name: treat-as-3 :)
(: Description: Evaluation of treat as expression where involving an integer data type and div expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := 100
return
($var treat as xs:integer) div 2
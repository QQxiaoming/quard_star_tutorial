(: Name: treat-as-13 :)
(: Description: Evaluation of treat as expression where involving an xs:integer data type and a heterogeneous sequence.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := 100
return
(xs:decimal($var) treat as xs:decimal, xs:double($var) treat as xs:double, xs:float($var) treat as xs:float, $var treat as xs:integer)
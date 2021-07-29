(: Name: treat-as-12 :)
(: Description: Evaluation of treat as expression where involving an xs:integer data type and a homogeneous sequence.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := 100
return
($var treat as xs:integer, $var treat as xs:integer, $var treat as xs:integer)
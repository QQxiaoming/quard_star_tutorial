(: Name: treat-as-6 :)
(: Description: Evaluation of treat as expression where involving an xs:float data types and abs function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:float(100)
return
fn:abs($var treat as xs:float)
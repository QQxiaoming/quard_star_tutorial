(: Name: treat-as-7 :)
(: Description: Evaluation of treat as expression where involving an xs:double data type and abs function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:double(100)
return
fn:abs($var treat as xs:double)
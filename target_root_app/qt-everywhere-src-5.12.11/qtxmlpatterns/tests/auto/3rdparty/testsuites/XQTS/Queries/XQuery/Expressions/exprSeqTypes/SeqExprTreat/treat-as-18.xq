(: Name: treat-as-18 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and double/float data types.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:double(-100)
return
fn:abs($var cast as xs:float treat as xs:float)
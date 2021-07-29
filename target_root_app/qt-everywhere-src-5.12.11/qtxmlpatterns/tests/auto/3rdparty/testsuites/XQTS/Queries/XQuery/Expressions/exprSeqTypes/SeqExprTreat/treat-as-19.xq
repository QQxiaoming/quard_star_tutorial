(: Name: treat-as-19 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and float/double data types.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:float(-100)
return
fn:abs($var cast as xs:double treat as xs:double)
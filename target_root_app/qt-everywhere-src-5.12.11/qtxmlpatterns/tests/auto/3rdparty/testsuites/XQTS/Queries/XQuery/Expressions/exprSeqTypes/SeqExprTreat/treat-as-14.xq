(: Name: treat-as-14 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and integer/float data types.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := -100
return
fn:abs($var cast as xs:float treat as xs:float)
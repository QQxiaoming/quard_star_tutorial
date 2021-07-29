(: Name: treat-as-16 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and float/integer data types.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:float(-100)
return
fn:abs($var cast as xs:integer treat as xs:integer)
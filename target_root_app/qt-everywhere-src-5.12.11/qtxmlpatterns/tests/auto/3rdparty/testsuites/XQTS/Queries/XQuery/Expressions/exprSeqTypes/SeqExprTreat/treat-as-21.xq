(: Name: treat-as-21 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and decimal/float data types.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:decimal(-100)
return
fn:abs($var cast as xs:float treat as xs:float)
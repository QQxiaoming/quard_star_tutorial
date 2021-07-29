(: Name: treat-as-20 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and decimal/double data types.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:decimal(-100)
return
fn:abs($var cast as xs:double treat as xs:double)
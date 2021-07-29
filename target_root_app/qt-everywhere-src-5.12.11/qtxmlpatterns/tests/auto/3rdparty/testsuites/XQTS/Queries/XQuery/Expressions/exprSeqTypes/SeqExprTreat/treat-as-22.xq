(: Name: treat-as-22 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and float/decimal data types (float cast as decimal treat as decimal).:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:float(-100)
return
fn:abs($var cast as xs:decimal treat as xs:decimal)
(: Name: treat-as-23 :)
(: Description: Evaluation of treat as expression with a "cast as" expression and double/decimal data types (double cast as decimal treat as decimal).:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:double(-100)
return
fn:abs($var cast as xs:decimal treat as xs:decimal)
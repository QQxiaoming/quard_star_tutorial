(: Name: treat-as-5 :)
(: Description: Evaluation of treat as expression where involving an decimal/integer data types and abs function. decimal treated as integer :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:decimal(100)
return
fn:abs($var cast as xs:integer treat as xs:integer)
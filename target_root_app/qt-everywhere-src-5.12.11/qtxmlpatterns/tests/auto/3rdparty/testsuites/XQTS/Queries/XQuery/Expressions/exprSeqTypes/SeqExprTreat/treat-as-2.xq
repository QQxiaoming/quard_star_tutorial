(: Name: treat-as-2 :)
(: Description: Evaluation of treat as expression where involving a string data type. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := "String 1"
return
fn:concat($var treat as xs:string,"String 2") 
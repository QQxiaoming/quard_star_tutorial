(: Name: treat-as-1 :)
(: Description: Evaluation of treat as expression where the dynamic type does not match expected type. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := (100+200) div 2
return
fn:concat($var treat as xs:string,"a string") 
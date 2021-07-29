(: Name: treat-as-8 :)
(: Description: Evaluation of treat as expression where involving an xs:boolean data type and fn:not function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:boolean(fn:true())
return
fn:not($var treat as xs:boolean)
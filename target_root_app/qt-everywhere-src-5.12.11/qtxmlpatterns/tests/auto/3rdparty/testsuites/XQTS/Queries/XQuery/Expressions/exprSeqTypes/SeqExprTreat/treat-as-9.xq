(: Name: treat-as-9 :)
(: Description: Evaluation of treat as expression where involving an xs:date data type and fn:year-from-date function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:date("2000-01-01+05:00")
return
fn:year-from-date($var treat as xs:date)
(: Name: treat-as-11 :)
(: Description: Evaluation of treat as expression where involving an xs:time data type and fn:hours-from-time function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:time("01:23:00+05:00")
return
fn:hours-from-time($var treat as xs:time)
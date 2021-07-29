(: Name: treat-as-10 :)
(: Description: Evaluation of treat as expression where involving an xs:dateTime data type and fn:minutes-from-dateTime function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := xs:dateTime("1999-05-31T13:20:00-05:00")
return
fn:minutes-from-dateTime($var treat as xs:dateTime)
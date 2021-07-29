(: Name: fn-dateTime-22 :)
(: Description: Evaluation of "fn:dateTime" as an argument to the function "adjust-dateTime-to-timezone". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:adjust-dateTime-to-timezone(fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")),xs:dayTimeDuration("PT10H"))
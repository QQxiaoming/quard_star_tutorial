(: Name: fn-dateTime-18 :)
(: Description: Evaluation of "fn:dateTime" as an argument to the function "hours-from-dateTime". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:hours-from-dateTime(fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")))

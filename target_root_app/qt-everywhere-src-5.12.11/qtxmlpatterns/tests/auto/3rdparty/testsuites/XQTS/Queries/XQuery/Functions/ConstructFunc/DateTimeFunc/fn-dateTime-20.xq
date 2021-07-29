(: Name: fn-dateTime-20 :)
(: Description: Evaluation of "fn:dateTime" as an argument to the function "seconds-from-dateTime". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:seconds-from-dateTime(fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")))

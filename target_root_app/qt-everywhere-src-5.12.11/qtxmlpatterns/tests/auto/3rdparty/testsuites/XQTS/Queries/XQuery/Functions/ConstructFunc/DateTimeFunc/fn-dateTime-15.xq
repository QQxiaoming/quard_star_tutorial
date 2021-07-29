(: Name: fn-dateTime-15 :)
(: Description: Evaluation of "fn:dateTime" as an argument to the function "year-from-dateTime". :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:year-from-dateTime(fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")))

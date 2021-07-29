(: Name: fn-dateTime-23 :)
(: Description: Evaluation of "fn:dateTime" as part of a subtraction operation. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")) - fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("22:00:00+10:00"))
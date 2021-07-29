(: Name: fn-dateTime-13 :)
(: Description: Evaluation of "fn:dateTime" function together together with the  op:dateTime-equal operator(gt). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")) gt fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00"))

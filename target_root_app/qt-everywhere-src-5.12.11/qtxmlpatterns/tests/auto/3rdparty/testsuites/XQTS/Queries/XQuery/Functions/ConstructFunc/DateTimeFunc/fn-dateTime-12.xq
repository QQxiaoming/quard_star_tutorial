(: Name: fn-dateTime-12 :)
(: Description: Evaluation of "fn:dateTime" function together together with the  op:dateTime-equal operator(le). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")) le fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00"))

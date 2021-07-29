(: Name: fn-dateTime-25 :)
(: Description: Evaluation of "fn:dateTime" used together with the "op:add-dayTimeDuration-to-dateTime" operator :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")) + xs:dayTimeDuration("P3DT1H15M")  
(: Name: fn-dateTime-26 :)
(: Description: Evaluation of "fn:dateTime" used together with the "subtract-yearMonthDuration-from-dateTime" operator :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")) - xs:yearMonthDuration("P1Y2M")
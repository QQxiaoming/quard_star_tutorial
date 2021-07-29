(: Name: fn-dateTime-28 :)
(: Description: Evaluation of "fn:dateTime" where first argument is empty sequence :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:dateTime((), xs:time("23:00:00+10:00")))
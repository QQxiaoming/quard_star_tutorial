(: Name: fn-dateTime-29 :)
(: Description: Evaluation of "fn:dateTime" where second argument is empty sequence :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:dateTime(xs:date("1999-12-31+10:00"), ()))
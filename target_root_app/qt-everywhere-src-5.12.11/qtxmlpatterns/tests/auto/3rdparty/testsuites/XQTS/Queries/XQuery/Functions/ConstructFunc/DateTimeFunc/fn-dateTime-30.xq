(: Name: fn-dateTime-30 :)
(: Description: Evaluation of "fn:dateTime" where both arguments are equal to the empty sequence :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:dateTime((),()))
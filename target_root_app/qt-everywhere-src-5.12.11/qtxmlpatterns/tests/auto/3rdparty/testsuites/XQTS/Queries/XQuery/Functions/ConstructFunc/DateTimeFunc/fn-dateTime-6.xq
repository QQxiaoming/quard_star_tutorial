(: Name: fn-dateTime-6 :)
(: Description: Evaluation of "fn:dateTime" function, where both arguments have the same timezone (+10:00). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31+10:00"), xs:time("23:00:00+10:00")) 

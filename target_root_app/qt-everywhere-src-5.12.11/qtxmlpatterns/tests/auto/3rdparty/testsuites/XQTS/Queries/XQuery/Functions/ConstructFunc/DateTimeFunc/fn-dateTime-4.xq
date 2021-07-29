(: Name: fn-dateTime-4 :)
(: Description: Evaluation of "fn:dateTime" function, where only the second argument have a timezone. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31"), xs:time("23:00:00Z")) 

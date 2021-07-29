(: Name: fn-dateTime-5 :)
(: Description: Evaluation of "fn:dateTime" function, where both arguments have the same timezone (Z). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31Z"), xs:time("23:00:00Z")) 

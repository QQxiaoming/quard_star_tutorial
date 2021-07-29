(: Name: fn-dateTime-3 :)
(: Description: Evaluation of "fn:dateTime" function, where only the first argument have a timezone. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31Z"), xs:time("23:00:00")) 

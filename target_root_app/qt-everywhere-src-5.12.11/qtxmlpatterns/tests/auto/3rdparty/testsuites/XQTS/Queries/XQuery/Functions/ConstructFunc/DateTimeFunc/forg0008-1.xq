(: Name: forg0008-1 :)
(: Description: Evaluation of "fn:dateTime" function, where the two arguments have different timezones. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31Z"), xs:time("12:00:00+10:00")) 

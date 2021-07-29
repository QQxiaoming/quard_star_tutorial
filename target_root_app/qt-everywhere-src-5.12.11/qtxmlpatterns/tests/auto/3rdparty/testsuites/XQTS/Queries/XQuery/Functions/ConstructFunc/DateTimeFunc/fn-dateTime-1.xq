(: Name: fn-dateTime-1 :)
(: Description: Evaluation of "fn:dateTime" function as per example 1 of the F & O Specs. for this function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:dateTime(xs:date("1999-12-31"), xs:time("12:00:00"))
(:Test: fn-implicit-timezone-23                             :)
(:Description: Evaluation of "fn:implicit-timezone" as an   :)
(: an argument to the adjust-dateTime-to-timezone function. :)                  

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00"),fn:implicit-timezone()))
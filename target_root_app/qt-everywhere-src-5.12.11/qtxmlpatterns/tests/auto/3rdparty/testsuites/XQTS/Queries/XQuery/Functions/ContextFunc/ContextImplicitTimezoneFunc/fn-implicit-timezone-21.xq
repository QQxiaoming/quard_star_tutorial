(:Test: fn-implicit-timezone-21                             :)
(:Description: Evaluation of "fn:implicit-timezone" as an   :)
(: an argument to the adjust-date-to-timezone function.     :)                  

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:adjust-date-to-timezone(xs:date("2000-10-30"),fn:implicit-timezone()))
(:Test: fn-implicit-timezone-22                             :)
(:Description: Evaluation of "fn:implicit-timezone" as an   :)
(: an argument to the adjust-time-to-timezone function.     :)                  

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:adjust-time-to-timezone(xs:time("10:00:00"),fn:implicit-timezone()))
(:Test: fn-implicit-timezone-15                             :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of an addition operation.  First operand is a call to xs:time function.:)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(xs:time("05:00:00") + fn:implicit-timezone())
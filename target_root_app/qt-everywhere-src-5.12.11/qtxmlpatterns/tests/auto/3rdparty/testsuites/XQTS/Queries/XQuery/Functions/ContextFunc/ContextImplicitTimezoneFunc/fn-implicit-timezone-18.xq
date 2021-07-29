(:Test: fn-implicit-timezone-18                             :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of an addition operation.  First operand is a call to xs:date function.:)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(xs:date("2000-10-30") + fn:implicit-timezone())
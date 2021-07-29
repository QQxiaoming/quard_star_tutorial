(:Test: fn-implicit-timezone-3                               :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of an addition operation.                            :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() + fn:implicit-timezone())
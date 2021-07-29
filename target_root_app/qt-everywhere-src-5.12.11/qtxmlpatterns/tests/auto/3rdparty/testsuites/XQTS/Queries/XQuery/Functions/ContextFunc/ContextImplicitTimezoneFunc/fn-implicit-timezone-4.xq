(:Test: fn-implicit-timezone-4                               :)
(:Description: Evaluation of "fn:implicit-timezone" as part  :)
(:of a subtraction operation.                                :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() - fn:implicit-timezone())
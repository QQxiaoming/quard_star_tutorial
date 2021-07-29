(:Test: fn-implicit-timezone-11                             :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of a division operation.  Second argument is 0.          :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() div  0 )
(:Test: fn-implicit-timezone-10                              :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of a division operation.  Second argument results in NaN :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() div ( 0 div 0E0))
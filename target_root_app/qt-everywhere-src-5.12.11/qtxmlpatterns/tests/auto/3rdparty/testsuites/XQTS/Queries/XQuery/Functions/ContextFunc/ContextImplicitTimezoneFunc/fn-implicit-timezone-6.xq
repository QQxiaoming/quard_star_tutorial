(:Test: fn-implicit-timezone-6                               :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(:of a multiplication operation.  Second argument is NaN:)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() * (0 div 0E0))
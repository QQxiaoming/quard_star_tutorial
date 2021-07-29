(:Test: fn-implicit-timezone-7                               :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of a multiplication operation.  Second argument is 0:)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() * 0)
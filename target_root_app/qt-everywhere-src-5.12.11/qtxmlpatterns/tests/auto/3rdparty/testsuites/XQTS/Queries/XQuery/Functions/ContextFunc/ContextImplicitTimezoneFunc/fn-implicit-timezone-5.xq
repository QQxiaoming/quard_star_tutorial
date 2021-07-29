(:Test: fn-implicit-timezone-5                               :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(:of a multiplication operation.                            :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() * xs:double(2))
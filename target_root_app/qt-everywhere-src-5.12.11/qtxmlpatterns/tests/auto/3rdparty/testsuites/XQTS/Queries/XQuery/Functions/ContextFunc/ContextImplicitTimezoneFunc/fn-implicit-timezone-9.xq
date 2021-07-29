(:Test: fn-implicit-timezone-9                               :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of a division operation.  :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() div xs:double(2))
(:Test: fn-implicit-timezone-13                             :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of a division operation.  Both operands includes the fn:implicit-timezone.:)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(implicit-timezone() + xs:dayTimeDuration('PT1S')) div
(implicit-timezone() + xs:dayTimeDuration('PT1S'))
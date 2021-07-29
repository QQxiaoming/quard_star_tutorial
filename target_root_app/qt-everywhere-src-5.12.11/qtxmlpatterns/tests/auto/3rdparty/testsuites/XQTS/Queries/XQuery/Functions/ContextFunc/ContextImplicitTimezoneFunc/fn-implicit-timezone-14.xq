(:Test: fn-implicit-timezone-14                             :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of a division operation.  Second operand is a call to xs:dayTimeDuration function.:)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone() div  xs:dayTimeDuration("P0DT60M00S"))
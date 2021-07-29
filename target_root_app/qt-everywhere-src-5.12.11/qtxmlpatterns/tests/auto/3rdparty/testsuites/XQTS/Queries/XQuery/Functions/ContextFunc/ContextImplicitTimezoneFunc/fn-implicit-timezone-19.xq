(:Test: fn-implicit-timezone-19                             :)
(:Description: Evaluation of "fn:implicit-timezone" as part :)
(: of a subtraction operation.  First operand is a call to xs:dateTime function.:)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(xs:dateTime("2000-10-30T11:12:00") - fn:implicit-timezone())
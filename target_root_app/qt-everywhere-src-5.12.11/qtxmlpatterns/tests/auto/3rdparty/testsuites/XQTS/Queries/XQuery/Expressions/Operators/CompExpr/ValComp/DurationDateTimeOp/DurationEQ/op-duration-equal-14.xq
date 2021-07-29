(: Name: op-duration-equal-14 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P36D" and used as argument to fn:not (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:not(xs:duration("P36D") eq xs:duration("P36D"))
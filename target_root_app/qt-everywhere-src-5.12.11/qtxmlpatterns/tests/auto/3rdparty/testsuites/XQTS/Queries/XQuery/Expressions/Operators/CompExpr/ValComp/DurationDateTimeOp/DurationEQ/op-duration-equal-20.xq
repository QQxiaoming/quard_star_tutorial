(: Name: op-duration-equal-20 :)
(: Description: Evaluation of duration-equal operator as part of boolean expression "or" operator and "fn:true" function (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:duration("P36D") eq xs:duration("P36D")) or fn:true()
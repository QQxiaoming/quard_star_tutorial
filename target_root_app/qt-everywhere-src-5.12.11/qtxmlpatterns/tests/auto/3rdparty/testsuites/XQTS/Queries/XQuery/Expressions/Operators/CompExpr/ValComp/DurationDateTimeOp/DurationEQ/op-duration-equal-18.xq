(: Name: op-duration-equal-18 :)
(: Description: Evaluation of duration-equal operator as part of boolean expression "and" operator and "fn:true" function (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:duration("P36D") eq xs:duration("P36D")) and fn:true()
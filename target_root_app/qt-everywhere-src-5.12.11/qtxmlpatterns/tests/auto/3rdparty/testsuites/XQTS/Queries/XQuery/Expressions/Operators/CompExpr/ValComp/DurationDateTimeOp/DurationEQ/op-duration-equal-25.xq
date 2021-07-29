(: Name: op-duration-equal-25 :)
(: Description: Evaluation of duration-equal operator as part of boolean expression "or" operator and "fn:false" function (uses ne operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:duration("P36D") ne xs:duration("P36D")) or fn:false()
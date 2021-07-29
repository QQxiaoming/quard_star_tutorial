(: Name: op-duration-equal-19 :)
(: Description: Evaluation of duration-equal operator as part of boolean expression "and" operator and "fn:true" function (uses ne operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:duration("P36D") ne xs:duration("P36D")) and fn:true()
(: Name: op-duration-equal-9 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P12M" (uses ne operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P12M") ne xs:duration("P12M") 
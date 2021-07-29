(: Name: op-duration-equal-5 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P1Y" (uses ne operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P1Y") ne xs:duration("P1Y") 
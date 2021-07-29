(: Name: op-duration-equal-11 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P365D" (uses ne operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P365D") ne xs:duration("P365D") 
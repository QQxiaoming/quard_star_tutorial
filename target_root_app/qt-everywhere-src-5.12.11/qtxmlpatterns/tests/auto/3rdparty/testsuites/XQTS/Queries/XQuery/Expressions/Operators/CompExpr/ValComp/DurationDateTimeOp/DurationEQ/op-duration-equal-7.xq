(: Name: op-duration-equal-7 :)
(: Description: Evaluation of duration-equal operator with both operands set to "PT24H" (uses ne operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("PT24H") ne xs:duration("PT24H") 
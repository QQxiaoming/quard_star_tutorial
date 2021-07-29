(: Name: op-duration-equal-6 :)
(: Description: Evaluation of duration-equal operator with both operands set to "PT24H" (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("PT24H") eq xs:duration("PT24H") 
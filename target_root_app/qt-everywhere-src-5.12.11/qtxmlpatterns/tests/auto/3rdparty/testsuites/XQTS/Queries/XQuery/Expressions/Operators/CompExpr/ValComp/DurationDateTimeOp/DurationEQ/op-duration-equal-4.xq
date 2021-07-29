(: Name: op-duration-equal-4 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P1Y" (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P1Y") eq xs:duration("P1Y") 
(: Name: op-duration-equal-8 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P12M" (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P12M") eq xs:duration("P12M") 
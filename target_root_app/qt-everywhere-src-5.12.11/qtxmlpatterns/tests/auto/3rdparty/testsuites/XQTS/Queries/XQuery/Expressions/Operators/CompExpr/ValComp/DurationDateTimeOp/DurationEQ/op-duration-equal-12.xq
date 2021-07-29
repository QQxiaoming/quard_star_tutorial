(: Name: op-duration-equal-12 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P36D" and "P39D" (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P36D") eq xs:duration("P39D") 
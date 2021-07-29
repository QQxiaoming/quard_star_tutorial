(: Name: op-duration-equal-10 :)
(: Description: Evaluation of duration-equal operator with both operands set to "P365D" (uses eq operator) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P365D") eq xs:duration("P365D") 
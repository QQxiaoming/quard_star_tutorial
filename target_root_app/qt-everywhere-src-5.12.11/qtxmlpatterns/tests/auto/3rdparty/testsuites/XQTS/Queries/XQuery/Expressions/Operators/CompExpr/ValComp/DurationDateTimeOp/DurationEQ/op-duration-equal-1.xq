(: Name: op-duration-equal-1 :)
(: Description: Evaluation of duration-equal operator as per example 1 :)
(: for this operator from Functions and Operators specs. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("P1Y") eq xs:duration("P12M")
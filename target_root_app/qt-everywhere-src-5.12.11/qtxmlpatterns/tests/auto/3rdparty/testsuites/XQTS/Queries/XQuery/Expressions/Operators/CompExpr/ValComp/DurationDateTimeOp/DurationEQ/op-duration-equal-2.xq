(: Name: op-duration-equal-2 :)
(: Description: Evaluation of duration-equal operator as per example 2 :)
(: for this operator from Functions and Operators specs. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:duration("PT24H") eq xs:duration("P1D")
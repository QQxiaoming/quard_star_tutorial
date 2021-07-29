(: Name: op-duration-equal-27 :)
(: Description: Evaluation of duration-equal operator as per example 5 for this :)
(: operator from the F and O specs. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:yearMonthDuration('P1Y') eq xs:dayTimeDuration('P365D')
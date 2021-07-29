(: Name: op-duration-equal-26 :)
(: Description: Evaluation of duration-equal operator as per example 4 for this :)
(: operator from the F and O specs. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:yearMonthDuration('P0Y') eq xs:dayTimeDuration('P0D')

(: Name: fn-replace-8:)
(: Description: Evaluation of replace function with pattern = "A+?" and replacement = "b" as an example 8 for this function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

replace("AAAA", "A+?", "b")
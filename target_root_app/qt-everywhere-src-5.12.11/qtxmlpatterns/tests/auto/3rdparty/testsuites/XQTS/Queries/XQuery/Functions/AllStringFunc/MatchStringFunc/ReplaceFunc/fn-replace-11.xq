(: Name: fn-replace-11:)
(: Description: Evaluation of fn:replace function with input set to empty sequence. :)
(: Uses the fn:count function to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:replace((), "bra", "*"))
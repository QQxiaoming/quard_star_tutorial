(: Name: fn-avg-2:)
(: Description: Evaluation of the fn:avg function with argument to empty sequence.:)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:avg(()))
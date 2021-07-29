(: Name: fn-nilled-22 :)
(: Description: Evaluation of nilled function with argument set to a processing instruction node :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:nilled(<?format role="output" ?>))
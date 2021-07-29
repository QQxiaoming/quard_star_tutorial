(: Name: fn-nilled-20 :)
(: Description: Evaluation of nilled function with argument set to an attribute node :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:nilled(attribute size {1}))
(: Name: fn-nilled-21 :)
(: Description: Evaluation of nilled function with argument set to a comment node :)
(: Uses fn:count to avoid empty file. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:count(fn:nilled(<!-- This is a comment node -->))
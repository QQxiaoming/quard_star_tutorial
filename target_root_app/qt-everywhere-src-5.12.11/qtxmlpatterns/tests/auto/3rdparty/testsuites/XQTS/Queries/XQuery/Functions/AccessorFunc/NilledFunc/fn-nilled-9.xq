(: Name: fn-nilled-9 :)
(: Description: Evaluation of nilled function with more than one argument. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled(<shoe xsi:nil="{fn:true()}"/>,"A Second Argument")
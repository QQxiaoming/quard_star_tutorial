(: Name: fn-nilled-7 :)
(: Description: Evaluation of nilled function with argument set to an element node with xsi:nill = fn:true(). :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled(<shoe xsi:nil="{fn:true()}"/>)
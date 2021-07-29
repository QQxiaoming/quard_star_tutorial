(: Name: fn-nilled-16 :)
(: Description: Evaluation of nilled function used as argument to fn:string function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:nilled(<shoe xsi:nil="{fn:true()}"/>))
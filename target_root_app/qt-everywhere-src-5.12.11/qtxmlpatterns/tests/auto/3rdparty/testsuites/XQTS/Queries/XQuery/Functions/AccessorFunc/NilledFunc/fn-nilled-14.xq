(: Name: fn-nilled-14 :)
(: Description: Evaluation of nilled function used a part of boolean expression ("or" operator and fn:true() :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled(<shoe xsi:nil="{fn:true()}"/>) or fn:true()
(: Name: fn-nilled-15 :)
(: Description: Evaluation of nilled function used a part of boolean expression ("or" operator and fn:false() :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled(<shoe xsi:nil="{fn:true()}"/>) or fn:false()
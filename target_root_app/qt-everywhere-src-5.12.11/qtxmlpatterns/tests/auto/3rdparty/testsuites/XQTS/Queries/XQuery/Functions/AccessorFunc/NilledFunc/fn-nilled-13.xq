(: Name: fn-nilled-13 :)
(: Description: Evaluation of nilled function used a part of boolean expression ("and" operator and fn:false() :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled(<shoe xsi:nil="{fn:true()}"/>) and fn:false()
(: Name: fn-nilled-12 :)
(: Description: Evaluation of nilled function used a part of boolean expression ("and" operator and fn:true() :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:nilled(<shoe xsi:nil="{fn:true()}"/>) and fn:true()
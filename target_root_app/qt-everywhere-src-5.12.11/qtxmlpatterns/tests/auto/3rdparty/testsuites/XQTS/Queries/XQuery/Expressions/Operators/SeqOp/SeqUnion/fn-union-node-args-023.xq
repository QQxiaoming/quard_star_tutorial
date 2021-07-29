(:*******************************************************:)
(: Test: fn-union-node-args-023.xq                       :)
(: Written By: Ravindranath Chennoju                     :)
(: Date: Tue May 24 03:34:54 2005                        :)
(: Purpose: arg: node & node                     :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $node in 
(($input-context//price/text()) , ($input-context//price/text())) union
(($input-context//price/text()) , ($input-context//price/text()))
return <a> {$node} </a>


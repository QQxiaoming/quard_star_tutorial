(:*******************************************************:) 
(: Test: fn-except-node-args-023.xq                       :) 
(: Written By: Ravindranath Chennoju                     :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: node & node                     :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external; 
(: insert-end :) 
 
(($input-context//price/text()) , ($input-context//price/text())) except 
($input-context//price/text())
 

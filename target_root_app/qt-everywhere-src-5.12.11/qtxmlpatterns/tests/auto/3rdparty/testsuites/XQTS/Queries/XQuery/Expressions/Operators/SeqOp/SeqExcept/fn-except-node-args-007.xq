(:*******************************************************:) 
(: Test: fn-except-node-args-007.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: comment node & node                     :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external; 
(: insert-end :) 
 
$input-context/comment() except $input-context/comment() 

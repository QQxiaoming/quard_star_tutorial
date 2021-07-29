(:*******************************************************:) 
(: Test: fn-except-node-args-006.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: processing-instruction node & node      :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external; 
(: insert-end :) 
 
$input-context/processing-instruction() except $input-context/processing-instruction(PI1)

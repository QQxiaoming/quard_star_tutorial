(:*******************************************************:) 
(: Test: fn-except-node-args-016.xq                       :) 
(: Written By: Ravindranath Chennoju                     :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg: node & node                     :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare namespace atomic="http://www.w3.org/XQueryTest"; 
declare variable $input-context1 external; 
(: insert-end :) 
 
($input-context1/atomic:root/atomic:integer) except ($input-context1/atomic:root/atomic:integer) 
 

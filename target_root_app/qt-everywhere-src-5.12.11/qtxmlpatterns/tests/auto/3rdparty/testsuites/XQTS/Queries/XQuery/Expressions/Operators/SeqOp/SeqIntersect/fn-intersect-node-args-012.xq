(:*******************************************************:) 
(: Test: fn-intersect-node-args-012.xq          :) 
(: Written By: Ravindranath Chennoju                             :) 
(: Date: Tue Jun 14 03:34:54 2005                        :) 
(: Purpose: arg:  node & non existing node                     :) 
(:*******************************************************:) 
 
(: insert-start :) 
declare variable $input-context external;
(: insert-end :) 
 
$input-context//author intersect $input-context//nonexisting 

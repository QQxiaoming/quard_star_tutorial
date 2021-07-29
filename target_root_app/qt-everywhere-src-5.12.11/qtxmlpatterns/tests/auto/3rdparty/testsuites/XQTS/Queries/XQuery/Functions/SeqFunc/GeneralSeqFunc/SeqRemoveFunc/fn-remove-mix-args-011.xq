(:*******************************************************:) 
(: Test: fn-remove-mix-args-011.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of float,integer,arg2: integer:) 
(:*******************************************************:) 
 
fn:remove( (xs:float("NaN"), 100, (), 2), 2) 

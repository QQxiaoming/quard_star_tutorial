(:*******************************************************:) 
(: Test: fn-remove-mix-args-013.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of double,float,boolean ,arg2: integer:) 
(:***************************************************************:) 
 
fn:remove( (xs:double("1.34"), xs:float("INF"), true()), 1) 

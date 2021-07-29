(:*******************************************************:) 
(: Test: fn-remove-mix-args-004.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of string, arg2: integer      :) 
(:*******************************************************:) 
 
fn:remove ( (xs:string("xyz"), (), (), "a" , "b"), 2) 

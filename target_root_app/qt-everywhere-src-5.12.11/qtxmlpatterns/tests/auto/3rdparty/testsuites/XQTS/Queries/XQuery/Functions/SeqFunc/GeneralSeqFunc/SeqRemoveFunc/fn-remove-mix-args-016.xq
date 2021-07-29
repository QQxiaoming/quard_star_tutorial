(:*******************************************************:) 
(: Test: fn-remove-mix-args-016.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of string,boolean, double  ,arg2: integer:) 
(:******************************************************************:) 
 
fn:remove( (xs:boolean("1"), xs:double("-INF"), "s"), 3) 

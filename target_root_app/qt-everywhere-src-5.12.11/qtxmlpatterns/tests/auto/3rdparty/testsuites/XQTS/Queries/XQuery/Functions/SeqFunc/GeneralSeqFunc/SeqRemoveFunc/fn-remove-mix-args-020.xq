(:*******************************************************:) 
(: Test: fn-remove-mix-args-020.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of time,decimal, integer ,arg2: integer:) 
(:****************************************************************:) 
 
fn:remove( (xs:time("12:30:00"), xs:decimal("2.000003"), 2), 2) 

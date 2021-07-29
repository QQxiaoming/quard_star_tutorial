(:*******************************************************:) 
(: Test: fn-remove-mix-args-012.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of string,float, decimal arg2: integer :) 
(:****************************************************************:) 
 
fn:remove( (xs:float("-INF"), xs:decimal("2.34"), "abc"), 2) 

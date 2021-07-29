(:*******************************************************:) 
(: Test: fn-remove-mix-args-018.xq           :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of string,boolean, date  ,arg2: integer:) 
(:****************************************************************:) 
 
fn:remove( (xs:boolean("true"), xs:date("1993-03-31"), 4, "a"),3) 

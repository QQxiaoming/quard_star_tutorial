(:*******************************************************:) 
(: Test: fn-remove-mix-args-019.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of string,dateTime,boolean   ,arg2: integer:) 
(:********************************************************************:) 
 
fn:remove( (xs:dateTime("1972-12-31T00:00:00"), xs:boolean("false"), (), (" ")) ,3) 

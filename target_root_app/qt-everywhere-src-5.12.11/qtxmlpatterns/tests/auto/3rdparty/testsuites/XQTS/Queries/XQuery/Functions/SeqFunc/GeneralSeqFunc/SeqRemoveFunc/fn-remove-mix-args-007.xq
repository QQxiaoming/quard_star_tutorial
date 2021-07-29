(:*******************************************************:) 
(: Test: fn-remove-mix-args-007.xq  :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of string,anyURI,arg2: integer:) 
(:*******************************************************:) 
 
fn:remove( (xs:anyURI("www.example.com"), "a", (""), "b"), -20) 

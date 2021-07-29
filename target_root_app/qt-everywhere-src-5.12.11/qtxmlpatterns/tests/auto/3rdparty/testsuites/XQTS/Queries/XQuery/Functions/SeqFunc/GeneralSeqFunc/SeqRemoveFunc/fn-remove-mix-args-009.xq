(:*******************************************************:) 
(: Test: fn-remove-mix-args-009.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:19:41 2005                        :) 
(: Purpose: arg1: sequence of decimal, integer, anyURI  arg2: integer  :) 
(:*********************************************************************:) 
 
fn:remove( (xs:decimal("1.01"), xs:integer("12"), xs:anyURI("www.example.com")),3) 

(:*******************************************************:) 
(: Test: fn-subsequence-mix-args-005.xq       :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:22:43 2005                        :) 
(: Purpose: arg1: sequence of string & anyURI, arg2 & arg3: integer  :) 
(:*******************************************************************:) 
 
fn:subsequence( ("a", xs:anyURI("www.example.com"),"b", "c"), 1, 3) 

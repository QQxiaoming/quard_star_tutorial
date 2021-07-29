(:*******************************************************:) 
(: Test: fn-subsequence-mix-args-022.xq       :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:22:43 2005                        :) 
(: Purpose: arg1: sequence of string & dateTime, arg2 & arg3: integer  :) 
(:*********************************************************************:) 
 
fn:subsequence ( ("a", xs:dateTime("1972-12-31T00:00:00"), "b", "c"), 0,2) 

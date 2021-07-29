(:*******************************************************:) 
(: Test: fn-subsequence-mix-args-023.xq       :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:22:43 2005                        :) 
(: Purpose: arg1: sequence of string & time   , arg2 & arg3: integer  :) 
(:********************************************************************:) 
 
fn:subsequence ( ("a", xs:time("12:30:00"), "b", "c"), 1, 2) 

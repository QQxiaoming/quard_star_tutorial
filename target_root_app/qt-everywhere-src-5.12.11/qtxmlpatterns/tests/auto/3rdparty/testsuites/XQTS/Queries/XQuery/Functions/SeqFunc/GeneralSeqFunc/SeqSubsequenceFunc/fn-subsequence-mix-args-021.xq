(:*******************************************************:) 
(: Test: fn-subsequence-mix-args-021.xq       :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:22:43 2005                        :) 
(: Purpose: arg1: sequence of string & date   , arg2 & arg3: integer  :) 
(:********************************************************************:) 
 
fn:subsequence ( ("a", xs:date("1993-03-31"), "b", "c"), 1,2) 

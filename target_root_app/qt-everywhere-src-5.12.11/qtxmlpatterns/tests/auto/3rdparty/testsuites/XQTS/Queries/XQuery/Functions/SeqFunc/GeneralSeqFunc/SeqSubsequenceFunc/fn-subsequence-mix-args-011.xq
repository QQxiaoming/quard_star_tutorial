(:*******************************************************:) 
(: Test: fn-subsequence-mix-args-011.xq       :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:22:43 2005                        :) 
(: Purpose: arg1: sequence of string & float  , arg2 & arg3: integer  :) 
(:********************************************************************:) 
 
fn:subsequence ( ("a", xs:float("NaN"), "b", "c"), 0, 2) 

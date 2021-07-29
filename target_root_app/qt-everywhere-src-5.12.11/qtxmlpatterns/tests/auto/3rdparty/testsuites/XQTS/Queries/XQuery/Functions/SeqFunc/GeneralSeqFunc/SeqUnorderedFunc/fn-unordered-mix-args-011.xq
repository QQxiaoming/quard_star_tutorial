(:*******************************************************:) 
(: Test: fn-unordered-mix-args-011.xq     :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:24:01 2005                        :) 
(: Purpose: arg: sequence of string & float              :) 
(:*******************************************************:) 
 
fn:unordered ( ("a", xs:float("NaN"), "b", "c")) 

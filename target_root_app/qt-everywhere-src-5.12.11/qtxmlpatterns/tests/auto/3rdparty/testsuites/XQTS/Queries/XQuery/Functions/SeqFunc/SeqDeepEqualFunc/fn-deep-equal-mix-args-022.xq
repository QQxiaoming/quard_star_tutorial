(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-022.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1:float, arg2:double values NaN :) 
(:*******************************************************:) 
 
fn:deep-equal( xs:float("NaN") , xs:double("NaN")) 

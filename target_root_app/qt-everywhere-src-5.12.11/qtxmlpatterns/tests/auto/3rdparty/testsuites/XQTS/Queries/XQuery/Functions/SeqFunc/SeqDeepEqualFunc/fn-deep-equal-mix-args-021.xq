(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-021.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1:float, arg2:double values -INF:) 
(:*******************************************************:) 
 
fn:deep-equal( xs:float("-INF") , xs:double("-INF")) 

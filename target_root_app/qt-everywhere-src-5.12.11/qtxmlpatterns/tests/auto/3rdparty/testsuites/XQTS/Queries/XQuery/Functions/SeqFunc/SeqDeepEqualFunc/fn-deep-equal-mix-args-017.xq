(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-017.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1:decimal , arg2:float          :) 
(:*******************************************************:) 
 
fn:deep-equal( xs:decimal(1.01) , xs:float(1.01)) 

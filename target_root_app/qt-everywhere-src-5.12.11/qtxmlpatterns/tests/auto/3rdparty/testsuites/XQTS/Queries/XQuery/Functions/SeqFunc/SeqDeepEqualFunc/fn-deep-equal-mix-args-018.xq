(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-018.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1:decimal, arg2:double          :) 
(:*******************************************************:) 
 
fn:deep-equal( xs:decimal(1.01) , xs:double(1.01)) 

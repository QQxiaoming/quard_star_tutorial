(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-027.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1:boolean arg2:value of false fn:) 
(:*******************************************************:) 
 
fn:deep-equal( xs:boolean("false") , false()) 

(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-029.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1:date, arg2: string            :) 
(:*******************************************************:) 
 
fn:deep-equal( xs:date("1993-03-31") , xs:string("1993-03-31")) 

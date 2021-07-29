(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-012.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1:anyURI and arg2:string        :) 
(:*******************************************************:) 
 
fn:deep-equal( xs:anyURI("www.example.com") , xs:string("www.example.com")) 

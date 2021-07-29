(:*******************************************************:) 
(: Test: fn-deep-equal-mix-args-011.xq                   :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 08:06:51 2005                        :) 
(: Purpose: Comparing arg1: anyURI, arg2: string         :) 
(:*******************************************************:) 
 
fn:deep-equal( xs:anyURI("www.example.com") , "www.example.com") 

(:*******************************************************:) 
(: Test: fn-unordered-mix-args-005.xq     :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:24:01 2005                        :) 
(: Purpose: arg: sequence of string & anyURI             :) 
(:*******************************************************:) 
 
fn:unordered ( ("a", xs:anyURI("www.example.com"), "b", "c")) 

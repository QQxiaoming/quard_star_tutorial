(:*******************************************************:) 
(: Test: fn-unordered-mix-args-007.xq     :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:24:01 2005                        :) 
(: Purpose: arg: sequence of string & integer            :) 
(:*******************************************************:) 
 
fn:unordered ( ("a", xs:integer("100"), xs:integer("-100"), "b", "c")) 

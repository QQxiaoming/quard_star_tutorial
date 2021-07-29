(:*******************************************************:) 
(: Test: fn-unordered-mix-args-008.xq     :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:24:01 2005                        :) 
(: Purpose: arg: sequence of string , decimal & integer  :) 
(:*******************************************************:) 
 
fn:unordered ( ("a", xs:decimal("-1.000000000001"), xs:integer("-100"), "b", "c")) 

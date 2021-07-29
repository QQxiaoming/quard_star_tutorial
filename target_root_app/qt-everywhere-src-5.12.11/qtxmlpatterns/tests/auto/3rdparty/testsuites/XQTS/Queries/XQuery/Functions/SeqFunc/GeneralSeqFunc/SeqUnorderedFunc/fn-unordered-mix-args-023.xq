(:*******************************************************:) 
(: Test: fn-unordered-mix-args-023.xq     :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:24:01 2005                        :) 
(: Purpose: arg: sequence of string & time               :) 
(:*******************************************************:) 
 
fn:unordered ( ("a", xs:time("12:30:00"), "b", "c")) 

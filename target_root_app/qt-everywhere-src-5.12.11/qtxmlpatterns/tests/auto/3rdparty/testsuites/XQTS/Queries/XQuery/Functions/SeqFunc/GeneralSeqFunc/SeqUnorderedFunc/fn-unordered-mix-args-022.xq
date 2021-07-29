(:*******************************************************:) 
(: Test: fn-unordered-mix-args-022.xq     :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Fri May 13 04:24:01 2005                        :) 
(: Purpose: arg: sequence of string & dateTime           :) 
(:*******************************************************:) 
 
fn:unordered ( ("a", xs:dateTime("1972-12-31T00:00:00"), "b", "c")) 

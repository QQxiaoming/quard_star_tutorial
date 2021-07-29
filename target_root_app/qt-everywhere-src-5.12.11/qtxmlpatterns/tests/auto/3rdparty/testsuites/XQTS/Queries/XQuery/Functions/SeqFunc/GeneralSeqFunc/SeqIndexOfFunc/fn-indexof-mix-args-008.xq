(:*******************************************************:) 
(: Test: fn-indexof-mix-args-008.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: arg1: Sequence of float , arg2: float        :) 
(:*******************************************************:) 
 
fn:index-of((xs:float('NaN')), fn:exactly-one(xs:float('NaN'))) 

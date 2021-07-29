(:*******************************************************:) 
(: Test: fn-indexof-mix-args-011.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: arg1: Sequence of double, arg2: double       :) 
(:*******************************************************:) 
 
 fn:index-of((xs:double('-INF')), fn:exactly-one(xs:double('-INF')))

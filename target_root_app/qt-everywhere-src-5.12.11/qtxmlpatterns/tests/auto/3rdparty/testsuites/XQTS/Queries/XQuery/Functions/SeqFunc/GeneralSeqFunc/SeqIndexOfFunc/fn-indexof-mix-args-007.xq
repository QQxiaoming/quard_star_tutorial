(:*******************************************************:) 
(: Test: fn-indexof-mix-args-007.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: arg1: empty Sequence , arg2: untypedAtomic   :) 
(:*******************************************************:) 
 
fn:index-of((),fn:exactly-one(xs:untypedAtomic(''))) 

(:*******************************************************:) 
(: Test: fn-indexof-mix-args-015.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: arg1: Sequence of negativeInteger, arg2: negativeInteger     :) 
(:***********************************************************************:) 
 
fn:index-of((xs:negativeInteger('-2'), xs:negativeInteger('-1')), fn:exactly-one(xs:negativeInteger('-1')))

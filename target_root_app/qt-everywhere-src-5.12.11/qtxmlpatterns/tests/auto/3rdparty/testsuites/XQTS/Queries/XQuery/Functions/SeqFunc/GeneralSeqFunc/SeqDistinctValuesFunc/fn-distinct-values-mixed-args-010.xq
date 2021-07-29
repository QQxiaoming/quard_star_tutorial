(:*******************************************************:) 
(: Test: fn-distinct-values-mixed-args-010.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Tue May 24 02:55:45 2005                        :) 
(: Purpose: arg: sequence of decimal                     :) 
(:*******************************************************:) 
 
 fn:distinct-values((xs:decimal('1.2'), xs:decimal('1.2000000000000001'))) 

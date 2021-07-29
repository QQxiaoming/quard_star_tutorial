(:*******************************************************:) 
(: Test: fn-distinct-values-mixed-args-012.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Tue May 24 02:55:45 2005                        :) 
(: Purpose: arg: sequence of decimal & float             :) 
(:*******************************************************:) 
 
 fn:distinct-values((xs:decimal('1.2'), xs:float('1.2'))) 

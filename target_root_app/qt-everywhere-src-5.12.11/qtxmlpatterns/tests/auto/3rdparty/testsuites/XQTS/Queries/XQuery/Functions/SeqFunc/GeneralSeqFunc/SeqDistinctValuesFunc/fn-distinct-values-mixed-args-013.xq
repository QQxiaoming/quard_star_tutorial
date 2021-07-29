(:*******************************************************:) 
(: Test: fn-distinct-values-mixed-args-013.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Tue May 24 02:55:45 2005                        :) 
(: Purpose: arg: sequence of decimal & double            :) 
(:*******************************************************:) 
 
 fn:distinct-values((xs:decimal('1.2'), xs:double('1.2'))) 

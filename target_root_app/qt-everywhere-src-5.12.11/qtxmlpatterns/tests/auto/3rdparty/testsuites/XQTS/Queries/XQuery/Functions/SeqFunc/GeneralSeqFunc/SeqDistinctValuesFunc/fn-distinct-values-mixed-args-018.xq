(:*******************************************************:) 
(: Test: fn-distinct-values-mixed-args-018.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Tue May 24 02:55:45 2005                        :) 
(: Purpose: arg: sequence of float & float               :) 
(:*******************************************************:) 
 
 fn:distinct-values((xs:float('-INF'), xs:float('INF'))) 

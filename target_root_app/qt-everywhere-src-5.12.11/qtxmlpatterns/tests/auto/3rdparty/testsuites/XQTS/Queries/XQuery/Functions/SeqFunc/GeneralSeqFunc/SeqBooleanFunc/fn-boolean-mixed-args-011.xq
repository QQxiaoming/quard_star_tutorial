(:*******************************************************:) 
(: Test: fn-boolean-mixed-args-011.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Mon May 23 04:50:42 2005                        :) 
(: Purpose: arg: float                                   :) 
(:*******************************************************:) 
 
<return> 
  { fn:boolean(xs:float('NaN')) } 
</return> 

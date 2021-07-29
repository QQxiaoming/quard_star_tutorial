(:*******************************************************:) 
(: Test: fn-boolean-mixed-args-045.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Mon May 23 04:50:42 2005                        :) 
(: Purpose: arg: int                                     :) 
(:*******************************************************:) 
 
<return> 
  { fn:boolean(xs:int('-2147483648')) } 
</return> 

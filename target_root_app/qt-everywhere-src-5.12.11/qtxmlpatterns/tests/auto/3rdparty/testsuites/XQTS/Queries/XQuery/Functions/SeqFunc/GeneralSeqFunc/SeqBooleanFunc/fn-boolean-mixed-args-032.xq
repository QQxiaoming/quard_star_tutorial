(:*******************************************************:) 
(: Test: fn-boolean-mixed-args-032.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Mon May 23 04:50:42 2005                        :) 
(: Purpose: arg: nonPositiveInteger                      :) 
(:*******************************************************:) 
 
<return> 
  { fn:boolean(xs:nonPositiveInteger('-99999999999999999')) } 
</return> 

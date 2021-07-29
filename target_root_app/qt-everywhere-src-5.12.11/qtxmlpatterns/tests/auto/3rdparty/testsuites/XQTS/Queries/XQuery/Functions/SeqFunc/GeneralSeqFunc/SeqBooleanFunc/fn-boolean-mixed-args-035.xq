(:*******************************************************:) 
(: Test: fn-boolean-mixed-args-035.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Mon May 23 04:50:42 2005                        :) 
(: Purpose: arg: negativeInteger                         :) 
(:*******************************************************:) 
 
<return> 
  { fn:boolean(xs:negativeInteger('-99999999999999999')) } 
</return> 

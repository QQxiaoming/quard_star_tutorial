(:*******************************************************:) 
(: Test: fn-boolean-mixed-args-024.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Mon May 23 04:50:42 2005                        :) 
(: Purpose: arg: decimal                                 :) 
(:*******************************************************:) 
 
<return> 
  { fn:boolean(xs:decimal('9.99999999999999999999999999')) } 
</return> 

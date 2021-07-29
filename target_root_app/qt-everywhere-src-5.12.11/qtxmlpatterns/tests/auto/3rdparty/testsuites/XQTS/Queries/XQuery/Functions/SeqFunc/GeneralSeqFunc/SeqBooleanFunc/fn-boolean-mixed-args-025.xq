(:*******************************************************:) 
(: Test: fn-boolean-mixed-args-025.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Mon May 23 04:50:42 2005                        :) 
(: Purpose: arg: decimal                                 :) 
(:*******************************************************:) 
 
<return> 
  { fn:boolean(xs:decimal('-123456789.123456789123456789')) } 
</return> 

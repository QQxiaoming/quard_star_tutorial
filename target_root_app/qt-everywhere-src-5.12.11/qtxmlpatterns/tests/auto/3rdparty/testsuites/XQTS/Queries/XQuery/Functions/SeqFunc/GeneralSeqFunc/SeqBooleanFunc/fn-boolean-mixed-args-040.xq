(:*******************************************************:) 
(: Test: fn-boolean-mixed-args-040.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Mon May 23 04:50:42 2005                        :) 
(: Purpose: arg: long                                    :) 
(:*******************************************************:) 
 
<return> 
  { fn:boolean(xs:long('-9223372036854775808')) } 
</return> 

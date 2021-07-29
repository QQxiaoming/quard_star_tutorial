(:*******************************************************:) 
(: Test: fn-indexof-mix-args-021.xq          :) 
(: Written By: Ravindranath Chennoju                     :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: Use empty string with integers in the sequence parameter :) 
(:***********************************************************************:) 
 
 fn:index-of( ("", 1, ""), "")

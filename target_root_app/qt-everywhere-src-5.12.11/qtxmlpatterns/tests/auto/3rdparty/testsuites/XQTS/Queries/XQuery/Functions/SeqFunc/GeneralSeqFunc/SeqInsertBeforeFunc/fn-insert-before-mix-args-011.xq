(:*******************************************************:) 
(: Test: fn-insert-before-mix-args-011.xq         :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:34:48 2005                        :) 
(: Purpose: arg1: Sequence of string, arg2:integer, arg3: float  :) 
(:***************************************************************:) 
 
 fn:insert-before( ("a", "b", "c"),2, xs:float("-INF")) 

(:*******************************************************:) 
(: Test: fn-insert-before-mix-args-018.xq         :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:34:48 2005                        :) 
(: Purpose: arg1: Sequence of string, arg2:integer, arg3: date   :) 
(:***************************************************************:) 
 
 fn:insert-before( ("a", "b", "c"),4, xs:date("1993-03-31")) 

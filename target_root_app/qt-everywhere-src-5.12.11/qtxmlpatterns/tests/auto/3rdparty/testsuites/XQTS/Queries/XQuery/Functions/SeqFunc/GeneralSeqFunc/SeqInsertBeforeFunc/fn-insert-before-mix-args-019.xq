(:*******************************************************:) 
(: Test: fn-insert-before-mix-args-019.xq         :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:34:48 2005                        :) 
(: Purpose: arg1: Sequence of string, arg2:integer, arg3: dateTime:) 
(:***************************************************************:) 
 
 fn:insert-before( ("a", "b", "c"),4, xs:dateTime("1972-12-31T00:00:00")) 

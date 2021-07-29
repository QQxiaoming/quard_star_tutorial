(:*******************************************************:) 
(: Test: fn-indexof-mix-args-012.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: arg1: Sequence of decimal, arg2: decimal     :) 
(:*******************************************************:) 
 
fn:index-of((xs:decimal('9.99999999999999999999999999')), fn:exactly-one(xs:decimal('9.99999999999999999999999999'))) 

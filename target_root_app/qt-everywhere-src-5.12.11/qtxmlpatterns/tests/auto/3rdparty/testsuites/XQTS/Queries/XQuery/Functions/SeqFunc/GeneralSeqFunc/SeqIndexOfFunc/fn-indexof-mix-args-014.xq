(:*******************************************************:) 
(: Test: fn-indexof-mix-args-014.xq          :) 
(: Written By: Pulkita Tyagi                             :) 
(: Date: Thu May 12 23:20:25 2005                        :) 
(: Purpose: arg1: Sequence of positiveInteger, arg2: positiveInteger     :) 
(:***********************************************************************:) 
 
 fn:index-of((xs:positiveInteger('1'),xs:positiveInteger('2')), fn:exactly-one(xs:positiveInteger('2'))) 

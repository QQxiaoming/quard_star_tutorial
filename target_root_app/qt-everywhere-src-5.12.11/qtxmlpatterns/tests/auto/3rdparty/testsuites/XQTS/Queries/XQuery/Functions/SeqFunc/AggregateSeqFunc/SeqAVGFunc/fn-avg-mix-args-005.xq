(:*******************************************************:)
(: Test: fn-avg-mix-args-005.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of integer, float                   :)
(:*******************************************************:)

fn:avg(( (3, 4, 5), xs:float('NaN') ))

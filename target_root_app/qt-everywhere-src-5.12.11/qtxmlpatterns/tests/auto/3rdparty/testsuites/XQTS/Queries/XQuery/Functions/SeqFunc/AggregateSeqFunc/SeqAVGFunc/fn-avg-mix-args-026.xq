(:*******************************************************:)
(: Test: fn-avg-mix-args-026.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of double,float                     :)
(:*******************************************************:)

fn:avg(( (xs:double("1.34"), xs:float("INF"))))

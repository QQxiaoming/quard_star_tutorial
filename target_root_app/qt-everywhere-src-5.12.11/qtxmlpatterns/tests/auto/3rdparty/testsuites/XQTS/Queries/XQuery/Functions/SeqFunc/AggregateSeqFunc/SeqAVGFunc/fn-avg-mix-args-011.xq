(:*******************************************************:)
(: Test: fn-avg-mix-args-011.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of integer,float,empty seq          :)
(:*******************************************************:)

fn:avg(( (xs:float("NaN"), 100, (), 2)))

(:*******************************************************:)
(: Test: fn-avg-mix-args-019.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of float,decimal                    :)
(:*******************************************************:)

fn:avg(( (xs:float("-INF"), xs:decimal("2.34"))))

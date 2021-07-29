(:*******************************************************:)
(: Test: fn-avg-mix-args-016.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of float,decimal                    :)
(:*******************************************************:)

fn:avg(( (xs:float("NaN"), xs:decimal("-999999999999999999")  )))

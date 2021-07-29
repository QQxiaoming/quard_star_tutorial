(:*******************************************************:)
(: Test: fn-avg-mix-args-012.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of float,decimal                    :)
(:*******************************************************:)

fn:avg(( (xs:float("-3.4028235E38"), xs:decimal("-999999999999999999") )))

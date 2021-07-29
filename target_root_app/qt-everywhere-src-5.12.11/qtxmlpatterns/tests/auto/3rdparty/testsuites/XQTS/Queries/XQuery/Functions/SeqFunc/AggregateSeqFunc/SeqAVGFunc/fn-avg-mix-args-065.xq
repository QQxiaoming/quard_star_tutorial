(:*******************************************************:)
(: Test: fn-avg-mix-args-065.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:time("12:30:00"), xs:decimal("2.000003"), 2)))

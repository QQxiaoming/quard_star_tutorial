(:*******************************************************:)
(: Test: fn-avg-mix-args-050.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:float("-INF"), xs:decimal("2.34"), "abc")))

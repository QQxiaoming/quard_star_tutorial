(:*******************************************************:)
(: Test: fn-avg-mix-args-040.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:decimal("617375191608514839"), "a", (), "3") ))

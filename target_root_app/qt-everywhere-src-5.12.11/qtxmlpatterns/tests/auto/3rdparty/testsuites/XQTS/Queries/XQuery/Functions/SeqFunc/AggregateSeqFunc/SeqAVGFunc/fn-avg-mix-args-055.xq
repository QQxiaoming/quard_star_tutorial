(:*******************************************************:)
(: Test: fn-avg-mix-args-055.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:double("-INF"), xs:integer("-999999999999999999") , "a", (), "3") ))

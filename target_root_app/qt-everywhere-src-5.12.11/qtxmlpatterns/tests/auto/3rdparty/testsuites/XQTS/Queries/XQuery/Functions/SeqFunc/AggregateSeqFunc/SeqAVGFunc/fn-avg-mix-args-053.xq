(:*******************************************************:)
(: Test: fn-avg-mix-args-053.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:double("1.7976931348623157E308"), xs:integer("-999999999999999999") , "a", (), "3") ))

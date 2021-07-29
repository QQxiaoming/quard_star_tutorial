(:*******************************************************:)
(: Test: fn-avg-mix-args-062.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:boolean("true"), xs:date("1993-03-31"), 4, "a")))

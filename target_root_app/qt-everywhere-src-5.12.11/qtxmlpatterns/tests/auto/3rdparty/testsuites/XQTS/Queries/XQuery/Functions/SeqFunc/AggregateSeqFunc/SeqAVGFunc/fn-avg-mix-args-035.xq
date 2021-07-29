(:*******************************************************:)
(: Test: fn-avg-mix-args-035.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:integer("100"), xs:string("abc"))))

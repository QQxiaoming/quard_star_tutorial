(:*******************************************************:)
(: Test: fn-avg-mix-args-063.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: Negative test gives FORG0006                 :)
(:*******************************************************:)

fn:avg(( (xs:date("1993-03-31"), xs:string("xyz"), (), (), "a" , "b")))

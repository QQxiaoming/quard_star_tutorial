(:*******************************************************:)
(: Test: fn-abs-more-args-032.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Oct 27 03:56:31 2005                        :)
(: Purpose: Negative Test gives FORG0001                 :)
(:*******************************************************:)

fn:abs(xs:int("NaN"))

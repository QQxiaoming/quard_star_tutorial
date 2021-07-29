(:*******************************************************:)
(: Test: fn-abs-more-args-082.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Thu Oct 27 03:56:31 2005                        :)
(: Purpose: Negative Test gives XPTY0004                 :)
(:*******************************************************:)

fn:abs(xs:boolean(fn:false()))

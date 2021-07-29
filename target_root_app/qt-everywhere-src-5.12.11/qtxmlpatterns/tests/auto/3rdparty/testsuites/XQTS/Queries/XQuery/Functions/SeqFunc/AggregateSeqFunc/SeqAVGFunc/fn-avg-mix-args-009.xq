(:*******************************************************:)
(: Test: fn-avg-mix-args-009.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of integer,decimal                  :)
(:*******************************************************:)

fn:avg( ( ( xs:decimal("-1.000000000001"), xs:integer("-100"))))

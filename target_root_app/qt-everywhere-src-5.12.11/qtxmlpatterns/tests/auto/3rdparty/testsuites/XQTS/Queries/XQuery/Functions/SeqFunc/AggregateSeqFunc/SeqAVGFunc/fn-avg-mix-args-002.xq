(:*******************************************************:)
(: Test: fn-avg-mix-args-002.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: yearMonthDuration                       :)
(:*******************************************************:)

fn:avg(( xs:yearMonthDuration("P20Y") , xs:yearMonthDuration("P10M") ))

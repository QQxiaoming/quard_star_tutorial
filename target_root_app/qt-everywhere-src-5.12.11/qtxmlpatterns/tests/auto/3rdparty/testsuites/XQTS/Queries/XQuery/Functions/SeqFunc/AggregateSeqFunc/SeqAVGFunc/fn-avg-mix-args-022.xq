(:*******************************************************:)
(: Test: fn-avg-mix-args-022.xq          :)
(: Written By: Pulkita Tyagi                             :)
(: Date: Tue Nov 22 05:19:47 2005                        :)
(: Purpose: arg: seq of double,integer                   :)
(:*******************************************************:)

fn:avg(( (xs:double("1.7976931348623157E308"), xs:integer("-999999999999999999") ) )) eq xs:double("8.988465674311579E307")

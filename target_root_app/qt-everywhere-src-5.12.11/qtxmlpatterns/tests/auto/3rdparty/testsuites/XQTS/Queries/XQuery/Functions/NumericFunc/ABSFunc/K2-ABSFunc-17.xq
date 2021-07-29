(:*******************************************************:)
(: Test: K2-ABSFunc-17                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:abs() on xs:long and check the return type. :)
(:*******************************************************:)
fn:abs(xs:long(-4)) instance of xs:integer
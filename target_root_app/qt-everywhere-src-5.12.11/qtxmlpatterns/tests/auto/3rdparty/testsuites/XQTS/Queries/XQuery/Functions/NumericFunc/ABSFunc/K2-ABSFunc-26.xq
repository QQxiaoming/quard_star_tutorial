(:*******************************************************:)
(: Test: K2-ABSFunc-26                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:abs() on xs:positiveInteger and check the return type. :)
(:*******************************************************:)
fn:abs(xs:positiveInteger(4)) instance of xs:integer
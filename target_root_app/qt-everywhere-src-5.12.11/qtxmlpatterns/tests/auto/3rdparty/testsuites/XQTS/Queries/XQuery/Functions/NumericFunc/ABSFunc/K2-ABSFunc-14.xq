(:*******************************************************:)
(: Test: K2-ABSFunc-14                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:abs() on xs:integer and check the return type. :)
(:*******************************************************:)
fn:abs(xs:integer(-4)) instance of xs:integer
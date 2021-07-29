(:*******************************************************:)
(: Test: K2-ABSFunc-16                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Invoke fn:abs() on xs:negativeInteger and check the return type. :)
(:*******************************************************:)
fn:abs(xs:negativeInteger(-4)) instance of xs:integer
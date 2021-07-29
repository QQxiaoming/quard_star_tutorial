(:*******************************************************:)
(: Test: K-SeqExprCast-518                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:double to xs:gDay isn't allowed. :)
(:*******************************************************:)
xs:double("3.3e3") cast as xs:gDay
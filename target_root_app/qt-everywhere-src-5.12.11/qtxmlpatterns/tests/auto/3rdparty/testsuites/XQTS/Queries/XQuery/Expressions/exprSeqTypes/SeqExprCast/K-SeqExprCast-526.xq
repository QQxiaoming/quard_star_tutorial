(:*******************************************************:)
(: Test: K-SeqExprCast-526                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting from xs:double to xs:hexBinary isn't allowed. :)
(:*******************************************************:)
xs:double("3.3e3") cast as xs:hexBinary
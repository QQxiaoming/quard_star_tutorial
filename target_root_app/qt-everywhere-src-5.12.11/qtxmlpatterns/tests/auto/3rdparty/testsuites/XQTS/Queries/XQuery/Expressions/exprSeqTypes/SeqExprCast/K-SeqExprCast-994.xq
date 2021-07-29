(:*******************************************************:)
(: Test: K-SeqExprCast-994                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gYearMonth to xs:hexBinary isn't allowed. :)
(:*******************************************************:)
xs:gYearMonth("1999-11") cast as xs:hexBinary
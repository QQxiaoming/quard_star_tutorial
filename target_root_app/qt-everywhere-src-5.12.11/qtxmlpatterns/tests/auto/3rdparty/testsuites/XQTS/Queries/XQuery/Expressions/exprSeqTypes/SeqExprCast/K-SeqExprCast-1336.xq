(:*******************************************************:)
(: Test: K-SeqExprCast-1336                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:hexBinary to xs:dayTimeDuration isn't allowed. :)
(:*******************************************************:)
xs:hexBinary("0FB7") cast as xs:dayTimeDuration
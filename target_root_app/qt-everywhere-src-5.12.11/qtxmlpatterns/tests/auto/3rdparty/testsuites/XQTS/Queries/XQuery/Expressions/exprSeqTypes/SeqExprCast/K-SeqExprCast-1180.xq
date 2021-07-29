(:*******************************************************:)
(: Test: K-SeqExprCast-1180                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gMonth to xs:dayTimeDuration isn't allowed. :)
(:*******************************************************:)
xs:gMonth("--11") cast as xs:dayTimeDuration
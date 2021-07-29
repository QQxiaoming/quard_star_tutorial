(:*******************************************************:)
(: Test: K-SeqExprCast-1146                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:gDay to xs:boolean isn't allowed. :)
(:*******************************************************:)
xs:gDay("---03") cast as xs:boolean
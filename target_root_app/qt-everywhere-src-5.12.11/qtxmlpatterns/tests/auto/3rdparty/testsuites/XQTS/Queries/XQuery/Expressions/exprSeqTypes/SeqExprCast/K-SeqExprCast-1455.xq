(:*******************************************************:)
(: Test: K-SeqExprCast-1455                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:QName to xs:gMonth isn't allowed. :)
(:*******************************************************:)
xs:QName("ncname") cast as xs:gMonth
(:*******************************************************:)
(: Test: K-SeqExprCast-1459                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:QName to xs:base64Binary isn't allowed. :)
(:*******************************************************:)
xs:QName("ncname") cast as xs:base64Binary
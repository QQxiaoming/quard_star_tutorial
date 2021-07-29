(:*******************************************************:)
(: Test: K-SeqExprCast-1272                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:base64Binary to xs:float isn't allowed. :)
(:*******************************************************:)
xs:base64Binary("aaaa") cast as xs:float
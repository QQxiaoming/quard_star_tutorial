(:*******************************************************:)
(: Test: K-SeqExprCast-1256                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Casting from xs:boolean to xs:anyURI isn't allowed. :)
(:*******************************************************:)
xs:boolean("true") cast as xs:anyURI
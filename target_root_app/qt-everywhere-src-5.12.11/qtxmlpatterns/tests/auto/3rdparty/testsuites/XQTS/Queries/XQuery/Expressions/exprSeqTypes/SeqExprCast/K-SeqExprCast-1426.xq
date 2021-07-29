(:*******************************************************:)
(: Test: K-SeqExprCast-1426                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:QName as source type and xs:string as target type should always evaluate to true. :)
(:*******************************************************:)
xs:QName("ncname") castable as xs:string
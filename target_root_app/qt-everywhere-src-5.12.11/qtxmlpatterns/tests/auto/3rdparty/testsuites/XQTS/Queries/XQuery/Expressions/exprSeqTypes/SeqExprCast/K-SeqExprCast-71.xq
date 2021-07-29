(:*******************************************************:)
(: Test: K-SeqExprCast-71                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Casting xs:untypedAtomic to xs:QName is an error. :)
(:*******************************************************:)
xs:untypedAtomic("ncname") cast as xs:QName?
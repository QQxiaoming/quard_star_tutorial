(:*******************************************************:)
(: Test: K-SeqExprCast-1009                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: 'castable as' involving xs:gYear as source type and xs:untypedAtomic as target type should always evaluate to true. :)
(:*******************************************************:)
xs:gYear("1999") castable as xs:untypedAtomic
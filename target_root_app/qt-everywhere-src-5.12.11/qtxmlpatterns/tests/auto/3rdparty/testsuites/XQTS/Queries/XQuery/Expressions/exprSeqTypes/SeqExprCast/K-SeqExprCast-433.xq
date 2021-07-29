(:*******************************************************:)
(: Test: K-SeqExprCast-433                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: 'castable as' involving xs:string as source type and xs:untypedAtomic as target type should always evaluate to true. :)
(:*******************************************************:)
xs:string("an arbitrary string") castable as xs:untypedAtomic
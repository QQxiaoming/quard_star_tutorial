(:*******************************************************:)
(: Test: K-SeqExprCast-421                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: 'castable as' involving xs:untypedAtomic as source type and xs:string as target type should always evaluate to true. :)
(:*******************************************************:)
xs:untypedAtomic("an arbitrary string(untypedAtomic source)") castable as xs:string
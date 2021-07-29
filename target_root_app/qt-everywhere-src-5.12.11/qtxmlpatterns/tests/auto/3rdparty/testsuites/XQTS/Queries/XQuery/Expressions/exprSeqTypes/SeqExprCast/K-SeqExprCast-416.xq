(:*******************************************************:)
(: Test: K-SeqExprCast-416                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "an arbitrary string(untypedAtomic source)" . :)
(:*******************************************************:)
xs:untypedAtomic(xs:untypedAtomic(
      "an arbitrary string(untypedAtomic source)"
    )) eq xs:untypedAtomic("an arbitrary string(untypedAtomic source)")
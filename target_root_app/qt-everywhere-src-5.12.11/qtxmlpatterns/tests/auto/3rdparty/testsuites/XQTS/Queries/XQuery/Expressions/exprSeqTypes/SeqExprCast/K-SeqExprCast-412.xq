(:*******************************************************:)
(: Test: K-SeqExprCast-412                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The xs:untypedAtomic constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:untypedAtomic(
      "an arbitrary string(untypedAtomic source)"
    ,
                                                     
      "an arbitrary string(untypedAtomic source)"
    )
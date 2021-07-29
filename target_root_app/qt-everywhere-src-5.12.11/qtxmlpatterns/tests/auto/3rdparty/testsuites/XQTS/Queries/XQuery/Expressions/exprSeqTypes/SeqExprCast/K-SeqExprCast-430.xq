(:*******************************************************:)
(: Test: K-SeqExprCast-430                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "an arbitrary string" . :)
(:*******************************************************:)
xs:string(xs:untypedAtomic(
      "an arbitrary string"
    )) eq xs:string("an arbitrary string")
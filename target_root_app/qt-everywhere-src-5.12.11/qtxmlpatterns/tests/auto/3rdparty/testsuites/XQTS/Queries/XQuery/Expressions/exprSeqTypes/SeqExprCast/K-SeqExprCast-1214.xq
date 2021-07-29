(:*******************************************************:)
(: Test: K-SeqExprCast-1214                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "true" . :)
(:*******************************************************:)
xs:boolean(xs:untypedAtomic(
      "true"
    )) eq xs:boolean("true")
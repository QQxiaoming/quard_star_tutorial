(:*******************************************************:)
(: Test: K-SeqExprCast-442                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "3.4e5" . :)
(:*******************************************************:)
xs:float(xs:untypedAtomic(
      "3.4e5"
    )) eq xs:float("3.4e5")
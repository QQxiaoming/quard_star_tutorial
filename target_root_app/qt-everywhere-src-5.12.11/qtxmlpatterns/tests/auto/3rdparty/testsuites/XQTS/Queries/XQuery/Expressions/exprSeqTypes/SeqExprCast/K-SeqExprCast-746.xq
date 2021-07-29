(:*******************************************************:)
(: Test: K-SeqExprCast-746                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "P3DT2H" . :)
(:*******************************************************:)
xs:dayTimeDuration(xs:untypedAtomic(
      "P3DT2H"
    )) eq xs:dayTimeDuration("P3DT2H")
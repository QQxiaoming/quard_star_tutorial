(:*******************************************************:)
(: Test: K-SeqExprCast-642                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "P1Y2M3DT10H30M" . :)
(:*******************************************************:)
xs:duration(xs:untypedAtomic(
      "P1Y2M3DT10H30M"
    )) eq xs:duration("P1Y2M3DT10H30M")
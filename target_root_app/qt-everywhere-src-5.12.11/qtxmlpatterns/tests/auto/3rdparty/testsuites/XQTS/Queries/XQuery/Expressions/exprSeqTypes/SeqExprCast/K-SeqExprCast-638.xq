(:*******************************************************:)
(: Test: K-SeqExprCast-638                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:duration constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:duration(
      "P1Y2M3DT10H30M"
    ,
                                                     
      "P1Y2M3DT10H30M"
    )
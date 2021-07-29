(:*******************************************************:)
(: Test: K-SeqExprCast-742                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:dayTimeDuration constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:dayTimeDuration(
      "P3DT2H"
    ,
                                                     
      "P3DT2H"
    )
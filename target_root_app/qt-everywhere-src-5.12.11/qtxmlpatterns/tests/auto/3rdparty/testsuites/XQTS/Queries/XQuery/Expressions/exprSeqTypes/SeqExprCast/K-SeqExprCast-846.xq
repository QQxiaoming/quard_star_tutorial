(:*******************************************************:)
(: Test: K-SeqExprCast-846                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:time constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:time(
      "03:20:00-05:00"
    ,
                                                     
      "03:20:00-05:00"
    )
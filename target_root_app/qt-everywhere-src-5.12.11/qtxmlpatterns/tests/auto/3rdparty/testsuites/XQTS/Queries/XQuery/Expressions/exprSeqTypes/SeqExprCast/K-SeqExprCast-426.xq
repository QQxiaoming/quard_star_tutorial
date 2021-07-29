(:*******************************************************:)
(: Test: K-SeqExprCast-426                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The xs:string constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:string(
      "an arbitrary string"
    ,
                                                     
      "an arbitrary string"
    )
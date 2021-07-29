(:*******************************************************:)
(: Test: K-SeqExprCast-534                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The xs:decimal constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:decimal(
      "10.01"
    ,
                                                     
      "10.01"
    )
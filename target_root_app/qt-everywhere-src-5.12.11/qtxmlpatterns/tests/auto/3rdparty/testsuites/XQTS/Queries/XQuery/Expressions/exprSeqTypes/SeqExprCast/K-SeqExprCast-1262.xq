(:*******************************************************:)
(: Test: K-SeqExprCast-1262                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: The xs:base64Binary constructor function must be passed exactly one argument, not two. :)
(:*******************************************************:)
xs:base64Binary(
      "aaaa"
    ,
                                                     
      "aaaa"
    )
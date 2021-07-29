(:*******************************************************:)
(: Test: K-SeqExprCast-490                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "3.3e3" . :)
(:*******************************************************:)
xs:double(xs:untypedAtomic(
      "3.3e3"
    )) eq xs:double("3.3e3")
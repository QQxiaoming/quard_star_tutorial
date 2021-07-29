(:*******************************************************:)
(: Test: K-SeqExprCast-538                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "10.01" . :)
(:*******************************************************:)
xs:decimal(xs:untypedAtomic(
      "10.01"
    )) eq xs:decimal("10.01")